// Provided interface
#include "KeyInt.h"

// Core (EventId_t: the set of valid keypad composites)
#include "Event.h"

// STL
#include <chrono>
#include <exception>
#include <stdexcept>
#include <iostream>
#include <unordered_set>

// Configuration
#include "config.h"

// Logging
#include "Logger.h"

/* Definition of static members */
int KeyInt::gpioInt_ = 0;
Pcf8575 *KeyInt::leftKbd_ = nullptr;
Pcf8575 *KeyInt::rightKbd_ = nullptr;
MsgQueue *KeyInt::outMsgQueue_ = nullptr;
int KeyInt::lastLeft_ = 0xFFFF;
int KeyInt::lastRight_ = 0xFFFF;
int KeyInt::debLeft_ = 0xFFFF;
int KeyInt::debRight_ = 0xFFFF;
uint32_t KeyInt::leftDebounceMs_[16] = {0};
uint32_t KeyInt::rightDebounceMs_[16] = {0};
bool KeyInt::seeded_ = false;
std::mutex KeyInt::stateMutex_;

/** @brief Polling fallback period in milliseconds (50 ms). */
static const uint32_t KEY_POLL_INTERVAL_MS = 50;

/**
 * @brief Debounce hold time in milliseconds. A port bit is accepted into the
 * debounced state only after it has held its new level for this long without
 * interrupting. The purpose is to filter mechanical/electrical contact bounce
 * (typically < ~20 ms); a 30 ms hold comfortably exceeds that while keeping
 * worst-case detection latency low (~80 ms with the 50 ms poll tick) so quick
 * taps are not swallowed. Longer-hold filtering of marginal-pin phantoms is
 * deliberately NOT done here: their levels are sustained and would pass any
 * short hold test — they are stopped by the VALID_KEY_IDS whitelist instead,
 * and the main phantom source (latch corruption from SMBus writes) has been
 * eliminated by the pure-read PCF8575 driver.
 */
static const uint32_t KEY_DEBOUNCE_MS = 30;

/**
 * @brief Set of composites that map to a real key (EventId_t, minus
 * E_NO_EVENT which is the no-key-pressed idle state).
 *
 * Debounce alone cannot stop phantom events from marginal pins: their
 * levels are sustained and settle one bit at a time over seconds, so each
 * flip passes the stability test and cascades into an emit storm. Only
 * composites that HmiLogic can act on are useful, so anything else is
 * dropped here before it reaches the message queue.
 */
static const std::unordered_set<uint32_t> VALID_KEY_IDS = {
    (uint32_t)E_KEY_0,      (uint32_t)E_KEY_1,     (uint32_t)E_KEY_2,
    (uint32_t)E_KEY_3,      (uint32_t)E_KEY_4,     (uint32_t)E_KEY_5,
    (uint32_t)E_KEY_6,      (uint32_t)E_KEY_7,     (uint32_t)E_KEY_8,
    (uint32_t)E_KEY_9,      (uint32_t)E_KEY_PLUS,  (uint32_t)E_KEY_MINUS,
    (uint32_t)E_KEY_VERB,   (uint32_t)E_KEY_NOUN,  (uint32_t)E_KEY_CLR,
    (uint32_t)E_KEY_KEYREL, (uint32_t)E_KEY_ENTR,  (uint32_t)E_KEY_RSET,
    (uint32_t)E_KEY_PRO};

/**
 * @brief Advance the per-bit debounce state for one 16-bit port.
 *
 * For each bit: if the raw level equals the currently accepted level the
 * pending timer is cleared (the bit is stable again). Otherwise a timer is
 * started on the first sighting and, once the bit has held the new level for
 * KEY_DEBOUNCE_MS continuously, the accepted state flips.
 *
 * @param raw [in] Raw 16-bit port reading
 * @param accepted [in,out] Debounced (accepted) port state
 * @param pendingMs [in,out] Per-bit timer: 0 = stable, else steady_clock ms
 * timestamp of the first sighting of the new level
 * @param now [in] Current steady_clock time in milliseconds
 */
static void debouncePort(int raw, int &accepted, uint32_t pendingMs[16],
                         uint32_t now)
{
  for (int bit = 0; bit < 16; ++bit)
  {
    const bool rawLevel = (raw & (1 << bit)) != 0;
    const bool accLevel = (accepted & (1 << bit)) != 0;

    if (rawLevel == accLevel)
    {
      // Raw matches the accepted level: bit is stable again
      pendingMs[bit] = 0;
      continue;
    }

    if (pendingMs[bit] == 0)
    {
      // First sighting of the new level: start the timer
      pendingMs[bit] = now;
    }
    else if ((now - pendingMs[bit]) >= KEY_DEBOUNCE_MS)
    {
      // The bit has held the new level long enough: accept it.
      // (unsigned subtraction wraps correctly mod 2^32 for monotonic clocks)
      accepted = rawLevel ? (accepted | (1 << bit)) : (accepted & ~(1 << bit));
      pendingMs[bit] = 0;
    }
  }
}

KeyInt::KeyInt(const int iicBus,
               const int iicAddrLeft, const int iicAddrRigth,
               const int gpioInt)
{
  // Save info locally
  gpioInt_ = gpioInt;

  // Initialize the PCF8575 controller
  leftKbd_ = new Pcf8575(iicBus, iicAddrLeft);
  rightKbd_ = new Pcf8575(iicBus, iicAddrRigth);

  // Restore both output latches to all-inputs (0xFFFF). Nothing else ever
  // writes the expanders, so without this a latch corrupted by an older
  // driver version (1-byte register writes latched on clone parts) would
  // persist across reboots and blind key sensing. A full 2-byte write of
  // 0xFFFF puts every pin into high-impedance input mode and self-heals.
  try {
    leftKbd_->writeLatch(0xFFFF);
    rightKbd_->writeLatch(0xFFFF);
  } catch (const std::exception &e) {
    LogWarning << "Fail restoring PCF8575 output latch: " << e.what()
               << std::endl;
  }

  // Set the interruption GPIO configuration
  gpioSetMode(gpioInt_, PI_INPUT);

  outMsgQueue_ = new MsgQueue(KEY_INT_MSG_QUEUE_FILE,
                              KEY_INT_MSG_QUEUE_ID, MSG_QUEUE_OUT);

  // Register callback for interruption
  gpioSetAlertFunc(gpioInt_, intHandler);

  /* Register the periodic polling fallback. It makes key detection robust
   * against a dead/intermittent INT line and against i2c NACKs that corrupt
   * a read at the exact moment of an edge (a missed poll tick self-corrects
   * on the next one, a missed edge is lost forever). Must be registered
   * after outMsgQueue_ exists. */
  if (gpioSetTimerFunc(0, KEY_POLL_INTERVAL_MS, pollHandler) < 0)
  {
    LogWarning << "Fail registering key polling timer" << std::endl;
  }
}

KeyInt::~KeyInt()
{
  // Remove message queue
  delete outMsgQueue_;
}

void KeyInt::intHandler(int gpio, int level, uint32_t tick)
{
  // Read the interruption line
  int intLine = gpioRead(gpioInt_);

  if (intLine == 0)
  {

    try
    {
      // Read data from the PCF8575
      int leftData = leftKbd_->read();
      int rightData = rightKbd_->read();

      processState(leftData, rightData);
    }
    catch (const std::exception &e)
    {
      LogWarning << "Fail accessing to the PCF8575" << std::endl;
    }
  }
}

void KeyInt::pollHandler(void)
{
  /* Polling fallback: catches key changes even when the INT line is open or
   * intermittent. Transient i2c failures (e.g. NACKs under LED load) are
   * skipped silently — the next tick re-reads and the state comparison
   * self-corrects, so a held key is still detected. */
  try
  {
    int leftData = leftKbd_->read();
    int rightData = rightKbd_->read();

    processState(leftData, rightData);
  }
  catch (const std::exception &e)
  {
    /* Transient i2c error: skip this tick */
  }
}

void KeyInt::processState(int leftData, int rightData)
{
  // Serialize the debounce state and the emit path (INT alert thread and
  // poll timer thread both reach here)
  std::lock_guard<std::mutex> lock(stateMutex_);

  const uint32_t now = static_cast<uint32_t>(
      std::chrono::duration_cast<std::chrono::milliseconds>(
          std::chrono::steady_clock::now().time_since_epoch())
          .count());

  if (!seeded_)
  {
    /* Seed the debounced state and the last-emitted state from the first
     * successful read, with all per-bit timers cleared. No startup emit:
     * whatever the idle level is (including stuck marginal pins) becomes
     * the reference, so it can never produce a spurious event. */
    debLeft_ = leftData;
    debRight_ = rightData;
    lastLeft_ = leftData;
    lastRight_ = rightData;
    for (int bit = 0; bit < 16; ++bit)
    {
      leftDebounceMs_[bit] = 0;
      rightDebounceMs_[bit] = 0;
    }
    seeded_ = true;
    return;
  }

  debouncePort(leftData, debLeft_, leftDebounceMs_, now);
  debouncePort(rightData, debRight_, rightDebounceMs_, now);

  // Only emit on a debounced state change (dedup between INT edge and poll
  // ticks)
  if ((debLeft_ == lastLeft_) && (debRight_ == lastRight_))
  {
    return;
  }

  lastLeft_ = debLeft_;
  lastRight_ = debRight_;

  /* Compose the composite key id once: left port in the high word, right
   * port in the low word. Cast to unsigned before shifting: a signed shift
   * into the sign bit is undefined behavior (debLeft_ can have bit 15 set). */
  const uint32_t keyId =
      static_cast<uint32_t>(debLeft_) << 16 | static_cast<uint32_t>(debRight_);

  /* Whitelist: drop composites that do not map to a real key. Phantom
   * interference levels settle slowly and pass the debounce stability test,
   * so without this filter they cascade into an emit storm. lastLeft_/
   * lastRight_ are already updated above, so later transitions from a
   * filtered state are still detected and re-evaluated. */
  if (VALID_KEY_IDS.find(keyId) == VALID_KEY_IDS.end())
  {
    LogDebug << "Filtered non-key composite: 0x" << std::hex << keyId
             << std::endl;
    return;
  }

  /* Compose message to send */
  KeyMsg_t msg;

  msg.mType = 1;
  msg.mData.keyId = keyId;
  msg.mData.keyModifier = KM_SHORT;

  std::cout << "KeyId: 0x" << std::hex << msg.mData.keyId << std::endl;

  /* Queue message for sending */
  outMsgQueue_->send(msg);
}
