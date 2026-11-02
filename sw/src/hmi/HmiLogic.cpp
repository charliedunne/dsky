#include "HmiLogic.h"

// Logging
#include "HmiData.h"
#include "Logger.h"

// Exceptions
#include "ExcInvalidCommand.h"

/**
 * @brief Function to resolve the falue of the key event
 *
 * @return key value
 */
int key(EventId_t);

HmiLogic::HmiLogic(VariableStore &store) : store_(store)
{
}

HmiLogic::~HmiLogic()
{
}

void HmiLogic::parseKeyEvents(std::vector<Event> events)
{

  keyEvents_ = events;
}

void HmiLogic::updateHmiData(HmiData &data)
{
  /* Process each queued key event sequentially. The state handlers read
   * keyEvents_[0], so the batch is reduced to one event per iteration. An
   * event may transit the status machine (e.g. start an action), in which
   * case later events in the same batch are dispatched against the new
   * state, exactly as if they had arrived on a later frame. */
  const std::vector<Event> pending = keyEvents_;
  for (size_t i = 0; i < pending.size(); ++i)
  {
    keyEvents_.assign(1, pending[i]);

    switch (status_())
    {

    case MODE_IDLE:
      idle(data);
      break;

    case MODE_V_INPUT:
      verbInput(data);
      break;

    case MODE_N_INPUT:
      nounInput(data);
      break;

    case MODE_VALUE_INPUT:
      valueInput(data);
      break;

    case MODE_PROG_INPUT:
      programInput(data);
      break;

    case MODE_RUN:
      run(data);
      break;

    case MODE_PROG_RUN:
      programRun(data);
      break;

    case MODE_ERROR:
      error(data);
      break;

    default:
      LogError << "Unknown Status Machine" << std::endl;
      break;
    }
  }

  /* Each key event must be processed exactly once per frame. The state
   * handlers above (verbInput/nounInput) may start an action in the same
   * frame; if the event were still queued when the MODE_RUN check below
   * calls run(), it would look like a "key press while a cancellable
   * action is running" and immediately cancel the just-started action */
  keyEvents_.clear();

  if (status_() == MODE_RUN)
  {
    run(data);
  }

  if (status_() == MODE_PROG_RUN)
  {
    programRun(data);
  }

  /* Drive the background-program start countdown, if active. Keys are
   * processed before this tick, so a cancel wins in the same frame */
  if (bgCountdownActive_)
  {
    bgCountdownTick(data);
  }

  /* Publish the data */
  // data = data_;
  // std::memcpy(data, &data_, sizeof(HmiData));
}

void HmiLogic::idle(HmiData &data)
{
  const EventId_t key = keyEvents_[0].getEvent();

  if (key == E_KEY_VERB)
  {
    LogTrace << "VERB" << std::endl;

    data.rSetVerbLabel(DRAW_ON);

    status_.transit(MODE_V_INPUT);
  }
  else if (key == E_KEY_PRO)
  {
    LogTrace << "PRO" << std::endl;

    data.rSetProgLabel(DRAW_ON);

    status_.transit(MODE_PROG_INPUT);
  }
  else if (key == E_KEY_RSET)
  {
    /* RSET clears any persistent left-LCD alarm (e.g. OP ERROR blink from an
     * action error), returning the display to the clean boot screen */
    LogTrace << "RSET in IDLE, clearing left LCD" << std::endl;

    data.resetLeftLcdData();
  }
  else if (data.getLeftLcdData().lVerbList == DRAW_ON)
  {
    /* + / - flip between the verb list and variable list pages (wraps around) */
    if (key == E_KEY_PLUS || key == E_KEY_MINUS)
    {
      const int cur = data.getLeftLcdData().lVerbListPage;
      const int next = (key == E_KEY_PLUS) ? cur + 1 : cur - 1;
      /* Log the wrapped page so the trace matches what is actually displayed */
      const int wrapped = ((next % 2) + 2) % 2;
      LogTrace << "Key pressed in IDLE, verb list page -> " << wrapped << std::endl;
      data.lSetVerbListPage(next);
    }
    else
    {
      /* Any other key returns to the clean boot screen (page resets to 0) */
      LogTrace << "Key pressed in IDLE, clear verb list" << std::endl;
      data.clearVerbList();
    }
  }
}

void HmiLogic::verbInput(HmiData &data)
{
  /* Copy the Event key received */
  EventId_t eventKey = keyEvents_[0].getEvent();

  switch (eventKey)
  {

  case E_KEY_ENTR:
  case E_KEY_NOUN:

    LogTrace << "Enter Verb input" << std::endl;

    try
    {
      if (actions_.needNoun(data.getRightLcdData().nVerb))
      {
        status_.transit(MODE_N_INPUT);
        data.rSetNounLabel(DRAW_ON);
        LogTrace << "Go no Noun input" << std::endl;
      }
      else
      {
        status_.transit(MODE_RUN);
        LogTrace << "Run action since Verb is enough [verb: " << data.getRightLcdData().nVerb << "]" << std::endl;
        run(data);
      }
    }
    catch (const ExcInvalidCommand &e)
    {
      LogTrace << "Invalid Verb command" << std::endl;
      data.lSetOpErr(DRAW_BLINK);
      status_.transit(MODE_ERROR);
    }

    break;

  case E_KEY_0:
  case E_KEY_1:
  case E_KEY_2:
  case E_KEY_3:
  case E_KEY_4:
  case E_KEY_5:
  case E_KEY_6:
  case E_KEY_7:
  case E_KEY_8:
  case E_KEY_9:

    if (data.getRightLcdData().nVerbMode == DRAW_OFF)
    {

      data.rSetnVerb(key(eventKey), DRAW_ON);
    }
    else
    {
      data.rSetnVerb(data.getRightLcdData().nVerb * 10 + key(eventKey),
                     DRAW_ON);
    }
    break;

  case E_KEY_VERB:
  case E_KEY_CLR:
    data.rSetnVerb(UNASIGNED, DRAW_OFF);
    break;

  case E_KEY_RSET:
    data.resetRightLcdData();
    data.resetLeftLcdData();
    status_.transit(MODE_IDLE);
    break;

  default:
    /* Keys not part of the input flow cancel back to IDLE */
    LogTrace << "Cancel verb input, move to IDLE" << std::endl;
    data.resetRightLcdData();
    status_.transit(MODE_IDLE);
    break;
  }
}

void HmiLogic::bgCountdownTick(HmiData &data)
{
  const auto now = std::chrono::steady_clock::now();
  const int remaining = 5 - static_cast<int>(std::chrono::duration_cast<std::chrono::seconds>(now - bgCountdownStart_).count());

  if (remaining <= 0)
  {
    /* Countdown complete: start the scheduled program and clear the display */
    LogTrace << "Background countdown complete, starting program [id: " << bgCountdownProgId_ << "]" << std::endl;
    bgCountdownActive_ = false;
    data.resetRightLcdData();

    try
    {
      Program &prog = programs_(bgCountdownProgId_);
      prog.start();
    }
    catch (const ExcInvalidCommand &e)
    {
      LogTrace << "Invalid Program command" << std::endl;
      data.lSetOpErr(DRAW_BLINK);
      status_.transit(MODE_ERROR);
      return;
    }

    status_.transit(MODE_IDLE);
  }
  else
  {
    data.rSetnR1(remaining, DRAW_ON);
  }
}

void HmiLogic::programInput(HmiData &data)
{
  /* Copy the Event key received */
  EventId_t eventKey = keyEvents_[0].getEvent();

  /* While a background program is counting down, any key cancels it. The key
   * itself is consumed by the post-loop keyEvents_.clear() and never reprocessed */
  if (bgCountdownActive_)
  {
    LogTrace << "Cancel background program countdown [id: " << bgCountdownProgId_ << "]" << std::endl;
    bgCountdownActive_ = false;
    data.resetRightLcdData();
    status_.transit(MODE_IDLE);
    return;
  }

  switch (eventKey)
  {

  case E_KEY_ENTR:

    LogTrace << "Enter Program input" << std::endl;

    try
    {
      /* Resolve the program first so an invalid ID throws before any state change */
      Program &prog = programs_(data.getRightLcdData().nProg);

      if (prog.foreground())
      {
        /* Foreground programs start immediately and own the HMI until they finish */
        prog.start();
        LogTrace << "Program started [id: " << data.getRightLcdData().nProg << "]" << std::endl;
        fgActive_ = static_cast<FgProgram *>(&prog);
        status_.transit(MODE_PROG_RUN);
      }
      else
      {
        /* Background programs are deferred behind a 5->0 downcount shown in R1.
         * Stay in MODE_PROG_INPUT: the tick hook drives the countdown and any
         * key press cancels it */
        bgCountdownActive_ = true;
        bgCountdownProgId_ = data.getRightLcdData().nProg;
        bgCountdownStart_ = std::chrono::steady_clock::now();
        data.rSetnR1(5, DRAW_ON);
        LogTrace << "Program scheduled [id: " << data.getRightLcdData().nProg << "], starting in 5s" << std::endl;
      }
    }
    catch (const ExcInvalidCommand &e)
    {
      LogTrace << "Invalid Program command" << std::endl;
      data.lSetOpErr(DRAW_BLINK);
      status_.transit(MODE_ERROR);
    }

    break;

  case E_KEY_0:
  case E_KEY_1:
  case E_KEY_2:
  case E_KEY_3:
  case E_KEY_4:
  case E_KEY_5:
  case E_KEY_6:
  case E_KEY_7:
  case E_KEY_8:
  case E_KEY_9:

    if (data.getRightLcdData().nProgMode == DRAW_OFF)
    {

      data.rSetnProg(key(eventKey), DRAW_ON);
    }
    else
    {
      data.rSetnProg(data.getRightLcdData().nProg * 10 + key(eventKey),
                     DRAW_ON);
    }
    break;

  case E_KEY_RSET:
  case E_KEY_CLR:
  default:
    /* Keys not part of the input flow cancel back to IDLE */
    LogTrace << "Cancel program input, move to IDLE" << std::endl;
    data.resetRightLcdData();
    status_.transit(MODE_IDLE);
    break;
  }
}

void HmiLogic::nounInput(HmiData &data)
{

  /* Copy the Event key received */
  EventId_t eventKey = keyEvents_[0].getEvent();

  switch (eventKey)
  {

  case E_KEY_ENTR:

    if (actions_.needValue(data.getRightLcdData().nVerb,
                           data.getRightLcdData().nNoun))
    {
      /* The command needs a value: prompt the user to type it */
      LogTrace << "Command needs a value, move to VALUE_INPUT" << std::endl;
      status_.transit(MODE_VALUE_INPUT);
      data.rSetR2Label(DRAW_BLINK);
      data.rSetR1Label(DRAW_ON);
      /* Show the "in" indicator while the user types the value.
       * No hold needed: no reap runs during VALUE_INPUT, so the
       * string stays until it is explicitly cleared or the right
       * LCD data is reset after the command completes. */
      data.rSetStrR1("in", DRAW_ON);
    }
    else
    {
      try
      {
        status_.transit(MODE_RUN);
        run(data);
      }
      catch (const ExcInvalidCommand &e)
      {
        LogWarning << "Invalid command set";
        data.lSetOpErr(DRAW_BLINK);
        status_.transit(MODE_ERROR);
      }
    }

    break;

  case E_KEY_0:
  case E_KEY_1:
  case E_KEY_2:
  case E_KEY_3:
  case E_KEY_4:
  case E_KEY_5:
  case E_KEY_6:
  case E_KEY_7:
  case E_KEY_8:
  case E_KEY_9:

    if (data.getRightLcdData().nNounMode == DRAW_OFF)
    {
      data.rSetnNoun(key(eventKey), DRAW_ON);
    }
    else
    {
      data.rSetnNoun(data.getRightLcdData().nNoun * 10 + key(eventKey),
                     DRAW_ON);
    }

    break;

  case E_KEY_VERB:
    data.rSetnVerb(UNASIGNED, DRAW_OFF);
    data.rSetnNoun(UNASIGNED, DRAW_OFF);
    data.rSetNounLabel(DRAW_OFF);
    status_.transit(MODE_V_INPUT);
    break;

  case E_KEY_NOUN:
  case E_KEY_CLR:
    data.rSetnNoun(UNASIGNED, DRAW_OFF);
    break;

  case E_KEY_RSET:
    data.resetRightLcdData();
    data.resetLeftLcdData();
    status_.transit(MODE_IDLE);
    break;

  default:
    /* Keys not part of the input flow cancel back to IDLE */
    LogTrace << "Cancel noun input, move to IDLE" << std::endl;
    data.resetRightLcdData();
    status_.transit(MODE_IDLE);
    break;
  }
}

void HmiLogic::valueInput(HmiData &data)
{
  /* Copy the Event key received */
  EventId_t eventKey = keyEvents_[0].getEvent();

  switch (eventKey)
  {

  case E_KEY_ENTR:

    if (data.getRightLcdData().nR2 == UNASIGNED)
    {
      /* No value typed yet, stay in the prompt */
      break;
    }

    status_.transit(MODE_RUN);
    run(data);
    break;

  case E_KEY_0:
  case E_KEY_1:
  case E_KEY_2:
  case E_KEY_3:
  case E_KEY_4:
  case E_KEY_5:
  case E_KEY_6:
  case E_KEY_7:
  case E_KEY_8:
  case E_KEY_9:

    if (store_.isBool(data.getRightLcdData().nNoun) && key(eventKey) != 0 && key(eventKey) != 1)
    {
      /* Bool variables accept strictly the digits 0 and 1 */
      LogTrace << "Reject digit for bool variable" << std::endl;
      break;
    }

    if (data.getRightLcdData().nR2Mode == DRAW_OFF)
    {
      data.rSetnR2(key(eventKey), DRAW_ON);
    }
    else if (data.getRightLcdData().nR2 < 100000)
    {
      /* Cap the value at 6 digits */
      data.rSetnR2(data.getRightLcdData().nR2 * 10 + key(eventKey), DRAW_ON);
    }

    break;

  case E_KEY_VERB:
    data.rSetnVerb(UNASIGNED, DRAW_OFF);
    data.rSetnNoun(UNASIGNED, DRAW_OFF);
    data.rSetNounLabel(DRAW_OFF);
    data.rSetnR2(UNASIGNED, DRAW_OFF);
    data.rSetR2Label(DRAW_OFF);
    /* Clear the "in" indicator: value entry is being abandoned */
    data.rSetStrR1("", DRAW_OFF);
    status_.transit(MODE_V_INPUT);
    break;

  case E_KEY_NOUN:
  case E_KEY_CLR:
    data.rSetnR2(UNASIGNED, DRAW_OFF);
    break;

  case E_KEY_RSET:
    data.resetRightLcdData();
    data.resetLeftLcdData();
    status_.transit(MODE_IDLE);
    break;

  default:
    /* Keys not part of the input flow cancel back to IDLE */
    LogTrace << "Cancel value input, move to IDLE" << std::endl;
    data.resetRightLcdData();
    status_.transit(MODE_IDLE);
    break;
  }
}

void HmiLogic::run(HmiData &data)
{
  /* Take verb and noun for easy readability */
  int verb = data.getRightLcdData().nVerb;
  int noun = data.getRightLcdData().nNoun;

  if (keyEvents_.size() > 0)
  {
    /* Copy the Event key received */
    EventId_t eventKey = keyEvents_[0].getEvent();

    LogTrace << "Key detected" << std::endl;

    /* A running cancellable action is aborted by ANY key press (incl. RSET).
     * cancel() is non-blocking, unlike stop(), so the HMI loop never freezes */
    Action &action = actions_(data);
    if (action.isRunning() && action.cancellable())
    {
      LogTrace << "Key detected while cancellable action running, cancelling" << std::endl;
      action.cancel();
      data.resetRightLcdData();
      data.resetLeftLcdData();
      status_.transit(MODE_IDLE);
      return;
    }

    switch (eventKey)
    {
    case E_KEY_RSET:
      actions_(data).stop();
      data.resetRightLcdData();
      data.resetLeftLcdData();
      status_.transit(MODE_IDLE);
      return;

    default:
      /* Forward the key to a running action that wants to handle it.
       * A periodic action keeps running after this, so no new action is
       * started below and the frame simply re-renders */
      if (action.isRunning())
      {
        action.handleKey(eventKey);
      }
      break;
    }
  }

  /* A finished single-shot action: always reap its thread, which resets
   * the finished flag that a non-blocking cancel() leaves stale. Go back
   * to IDLE unless this frame is starting a new command (ENTR/NOUN from
   * the input states), in which case fall through and start it below so
   * an immediate retry after a cancel is not swallowed by the reap */
  if (actions_(data).finished())
  {
    LogTrace << "Action finished, reaping" << std::endl;
    actions_(data).stop();

    bool newCommand = keyEvents_.size() == 1 &&
        (keyEvents_[0].getEvent() == E_KEY_ENTR ||
         keyEvents_[0].getEvent() == E_KEY_NOUN);

    if (!newCommand)
    {
      data.resetRightLcdData();
      status_.transit(MODE_IDLE);
      return;
    }
  }

  if (!actions_(data).isRunning())
  {
    LogTrace << "Running action..." << std::endl;

    // A new (non-VERB 0) command dismisses the VERB 0 menu
    if (verb != 0) {
      data.clearVerbList();
    }

    actions_(data).run();
  }
}

void HmiLogic::programRun(HmiData &data)
{
  /* The foreground program currently owning the HMI must be valid; if it is
   * missing something went wrong, so bail back to IDLE */
  if (fgActive_ == nullptr)
  {
    LogError << "No foreground program active in MODE_PROG_RUN" << std::endl;
    data.resetRightLcdData();
    data.resetLeftLcdData();
    status_.transit(MODE_IDLE);
    return;
  }

  if (keyEvents_.size() > 0)
  {
    /* Copy the Event key received */
    EventId_t eventKey = keyEvents_[0].getEvent();

    LogTrace << "Key detected in foreground program" << std::endl;

    /* A running cancellable foreground program is aborted by ANY key press.
     * cancel() is non-blocking, unlike stop(), so the HMI loop never freezes */
    if (fgActive_->isRunning() && fgActive_->cancellable())
    {
      LogTrace << "Key detected while cancellable foreground program running, cancelling" << std::endl;
      fgActive_->cancel();
      data.resetRightLcdData();
      data.resetLeftLcdData();
      status_.transit(MODE_IDLE);
      return;
    }

    /* Otherwise forward the key to the running foreground program */
    if (fgActive_->isRunning())
    {
      fgActive_->pushKey(eventKey);
    }
  }

  /* A finished foreground program: release the HMI. stop() is a safe no-op
   * join-wise here because the worker already cleared running_ on natural
   * exit, so the thread is reaped later by start() or the destructor */
  if (!fgActive_->isRunning())
  {
    LogTrace << "Foreground program finished, releasing HMI" << std::endl;
    fgActive_->stop();
    fgActive_ = nullptr;
    data.resetRightLcdData();
    data.resetLeftLcdData();
    status_.transit(MODE_IDLE);
  }
}

void HmiLogic::error(HmiData &data)
{
  /* Any key press clears the error and returns to the clean IDLE state */
  LogTrace << "Error cleared, move to IDLE" << std::endl;
  data.resetRightLcdData();
  data.resetLeftLcdData();
  status_.transit(MODE_IDLE);
}

void HmiLogic::registerAction(Action &action)
{
  actions_.registerAction(action);
}

ListOfActions &HmiLogic::getActions()
{
  return actions_;
}

void HmiLogic::registerProgram(Program &program)
{
  programs_.registerProgram(program);
}

ListOfPrograms &HmiLogic::getPrograms()
{
  return programs_;
}

int key(EventId_t id)
{

  /* Return value */
  int value = 0;

  switch (id)
  {

  case E_KEY_0:
    value = 0;
    break;

  case E_KEY_1:
    value = 1;
    break;

  case E_KEY_2:
    value = 2;
    break;

  case E_KEY_3:
    value = 3;
    break;

  case E_KEY_4:
    value = 4;
    break;

  case E_KEY_5:
    value = 5;
    break;

  case E_KEY_6:
    value = 6;
    break;

  case E_KEY_7:
    value = 7;
    break;

  case E_KEY_8:
    value = 8;
    break;

  case E_KEY_9:
    value = 9;
    break;

  default:
    return -1;
  }

  return value;
}
