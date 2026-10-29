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
int KeyInt::debLeft_ = 0xFFFF;
int KeyInt::debRight_ = 0xFFFF;
uint32_t KeyInt::leftDebounceMs_[16] = {0};
uint32_t KeyInt::rightDebounceMs_[16] = {0};
uint32_t KeyInt::leftPressMs_[16] = {0};
uint32_t KeyInt::rightPressMs_[16] = {0};
bool KeyInt::seeded_ = false;
std::mutex KeyInt::stateMutex_;

/** @brief Polling fallback period in milliseconds (50 ms). */
static const uint32_t KEY_POLL_INTERVAL_MS = 50;

/**
 * @brief Debounce hold time in milliseconds. A port bit is accepted into the
 * debounced state only after it has held its new level for this long without
 * interrupting. The purpose is to filter mechanical/electrical contact bounce
 * (typically < ~20 ms); a 20 ms hold sits right at that upper bound, which is
 * enough to reject bounce while keeping worst-case detection latency as low as
 * possible (~70 ms with the 50 ms poll tick) so even quick taps are not
 * swallowed. Longer-hold filtering of marginal-pin phantoms is
 * deliberately NOT done here: their levels are sustained and would pass any
 * short hold test — they are stopped by release-confirmed hold-duration
 * gating (KEY_MIN_PRESS_MS / KEY_MAX_HOLD_MS) in handlePortTransitions(),
 * and the main phantom source (latch corruption from SMBus writes) has been
 * eliminated by the pure-read PCF8575 driver.
 */
static const uint32_t KEY_DEBOUNCE_MS = 20;

/**
 * @brief Minimum press duration in milliseconds for a release-confirmed tap.
 * A real key press must hold its line low at least this long (after debounce)
 * to be emitted on release; shorter transitions are treated as noise.
 */
static const uint32_t KEY_MIN_PRESS_MS = 20;

/**
 * @brief Maximum press duration in milliseconds for a release-confirmed tap.
 * A human operator never holds a key this long; a marginal pin that stays
 * low for seconds (sustained interference) exceeds it, so its "release" is
 * suppressed instead of cascading into phantom events.
 */
static const uint32_t KEY_MAX_HOLD_MS = 1000;

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
    /* Seed the debounced state from the first successful read, with all
     * per-bit timers cleared and no press-start timestamps. No startup
     * emit: whatever the idle level is (including stuck marginal pins)
     * becomes the reference, so it can never produce a spurious event —
     * a later release of a pin that was already down at seed has no
     * recorded press start and is skipped. */
    debLeft_ = leftData;
    debRight_ = rightData;
    for (int bit = 0; bit < 16; ++bit)
    {
      leftDebounceMs_[bit] = 0;
      rightDebounceMs_[bit] = 0;
      leftPressMs_[bit] = 0;
      rightPressMs_[bit] = 0;
    }
    seeded_ = true;
    return;
  }

  /* Capture the previous debounced levels before advancing. Transitions are
   * detected on the debounced state itself, so the INT edge path and the
   * poll tick path see identical transitions and can never double-emit. */
  const int prevLeft = debLeft_;
  const int prevRight = debRight_;

  debouncePort(leftData, debLeft_, leftDebounceMs_, now);
  debouncePort(rightData, debRight_, rightDebounceMs_, now);

  handlePortTransitions(prevLeft, debLeft_, leftPressMs_, true, now);
  handlePortTransitions(prevRight, debRight_, rightPressMs_, false, now);
}

void KeyInt::handlePortTransitions(int prevPort, int curPort,
                                   uint32_t pressMs[16], bool isLeft,
                                   uint32_t now)
{
  for (int bit = 0; bit < 16; ++bit)
  {
    const bool prevLevel = (prevPort & (1 << bit)) != 0;
    const bool curLevel = (curPort & (1 << bit)) != 0;

    if (prevLevel == curLevel)
    {
      continue; // No transition on this bit
    }

    if (!curLevel)
    {
      /* idle -> pressed: a key press clears the port bit. Record when the
       * debounced level went to pressed so the release can be confirmed by
       * its hold duration. */
      pressMs[bit] = now;
      continue;
    }

    /* pressed -> idle (release). Confirm the press before emitting. */
    const uint32_t start = pressMs[bit];
    pressMs[bit] = 0;

    if (start == 0)
    {
      /* The bit was already down at seed (stuck marginal pin): no tracked
       * press, so no event. */
      continue;
    }

    const uint32_t duration = now - start; // wraps correctly mod 2^32
    if ((duration < KEY_MIN_PRESS_MS) || (duration > KEY_MAX_HOLD_MS))
    {
      /* Too short: noise. Too long: a sustained marginal-pin phantom whose
       * level is settling, not a human tap — suppress it. */
      LogDebug << "Suppressed key release: bit " << std::dec << bit
               << " held " << duration << " ms" << std::endl;
      continue;
    }

    /* Reconstruct the pre-release composite: this bit still cleared, all
     * other bits at their current debounced levels. Cast to unsigned before
     * shifting: a signed shift into the sign bit is undefined behavior. */
    const int preReleasePort = curPort & ~(1 << bit);
    const uint32_t keyId = isLeft
        ? (static_cast<uint32_t>(preReleasePort) << 16 |
           static_cast<uint32_t>(debRight_))
        : (static_cast<uint32_t>(debLeft_) << 16 |
           static_cast<uint32_t>(preReleasePort));

    /* Whitelist: drop composites that do not map to a real key. */
    if (VALID_KEY_IDS.find(keyId) == VALID_KEY_IDS.end())
    {
      LogDebug << "Filtered non-key composite: 0x" << std::hex << keyId
               << std::endl;
      continue;
    }

    sendKey(keyId);
  }
}

void KeyInt::sendKey(uint32_t keyId)
{
  /* Compose message to send */
  KeyMsg_t msg;

  msg.mType = 1;
  msg.mData.keyId = keyId;
  msg.mData.keyModifier = KM_SHORT;

  std::cout << "KeyId: 0x" << std::hex << msg.mData.keyId << std::endl;

  /* Queue message for sending */
  outMsgQueue_->send(msg);
}
