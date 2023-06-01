#include "HmiLogic.h"

#include "Logger.h"

/**
 * @brief Function to resolve the falue of the key event
 *
 * @return key value
 */
int key(EventId_t);

HmiLogic::HmiLogic()
{
  resetHmi();
}

HmiLogic::~HmiLogic()
{
}

void HmiLogic::parseKeyEvents(std::vector<Event> events)
{

  keyEvents_ = events;
}

void HmiLogic::updateHmiData(HmiData *data)
{

  // LogDebug << "Current Status: " << status_.to_string() << std::endl;

  if (keyEvents_.size() == 1)
  {

    LogDebug << "New key pressed" << std::endl;

    switch (status_())
    {

    case MODE_IDLE:
      idle();
      break;

    case MODE_V_INPUT:
      verbInput();
      break;

    case MODE_N_INPUT:
      nounInput();
      break;

    case MODE_RUN:
      run(data->nVerb, data->nNoun);
      break;

    case MODE_ERROR:
      error();
      break;

    default:
      LogError << "Unknown Status Machine" << std::endl;
      break;
    }
  }
  else if (keyEvents_.size() > 1)
  {

    /**
     * @todo What to do for multiple events at once
     */

    LogWarning << "@TODO Handle more than one event at one" << std::endl;
  }

  if (status_() == MODE_RUN)
  {
    run(data_.nVerb, data_.nNoun);
  }

  /* Publish the data */
  std::memcpy(data, &data_, sizeof(HmiData));
}

void HmiLogic::idle()
{
  if (keyEvents_[0].getEvent() == E_KEY_VERB)
  {
    LogTrace << "VERB" << std::endl;

    verbHmi();

    status_.transit(MODE_V_INPUT);
  }
}

void HmiLogic::verbInput()
{
  /* Copy the Event key received */
  EventId_t eventKey = keyEvents_[0].getEvent();

  switch (eventKey)
  {

  case E_KEY_ENTR:
  case E_KEY_NOUN:

    LogTrace << "Enter Verb input" << std::endl;

    if (actions_.needNoun(data_.nVerb))
    {
      nounHmi();
      status_.transit(MODE_N_INPUT);
      LogTrace << "Go no Noun input" << std::endl;
    }
    else
    {
      data_.nNoun = -1;
      status_.transit(MODE_RUN);
      LogTrace << "Run action since Verb is enough [verb: " << data_.nVerb << "]" << std::endl;
      run(data_.nVerb, data_.nNoun);
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

    if (data_.nVerbMode == DRAW_OFF)
    {

      data_.nVerbMode = DRAW_ON;
      data_.nVerb = key(eventKey);
    }
    else
    {
      data_.nVerb = data_.nVerb * 10 + key(eventKey);
    }
    break;

  case E_KEY_VERB:
  case E_KEY_CLR:
    verbHmi();
    break;

  case E_KEY_RSET:
    resetHmi();
    status_.transit(MODE_IDLE);
    break;

  default:
    LogWarning << "Not available operation" << std::endl;
    break;
  }
}

void HmiLogic::nounInput()
{

  /* Copy the Event key received */
  EventId_t eventKey = keyEvents_[0].getEvent();

  /* Set-up the labels for Verb and disable Noun */
  data_.lVerbMode = DRAW_ON;
  data_.lNounMode = DRAW_ON;

  switch (eventKey)
  {

  case E_KEY_ENTR:

    status_.transit(MODE_RUN);
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

    if (data_.nNounMode == DRAW_OFF)
    {
      data_.nNounMode = DRAW_ON;
      data_.nNoun = key(eventKey);
    }
    else
    {
      data_.nNoun = data_.nNoun * 10 + key(eventKey);
    }

    break;

  case E_KEY_VERB:
    verbHmi();
    status_.transit(MODE_V_INPUT);

  case E_KEY_NOUN:
  case E_KEY_CLR:
    nounHmi();
    break;

  case E_KEY_RSET:
    resetHmi();
    status_.transit(MODE_IDLE);
    break;

  default:
    LogWarning << "Not available operation" << std::endl;
    break;
  }
}

void HmiLogic::run(int verb, int noun)
{
  if (keyEvents_.size() > 0)
  {

    /* Copy the Event key received */
    EventId_t eventKey = keyEvents_[0].getEvent();

    LogTrace << "Key detected" << std::endl;

    switch (eventKey)
    {
    case E_KEY_RSET:
      actions_(verb, noun)->stop();
      resetHmi();
      status_.transit(MODE_IDLE);
      break;

    default:
      break;
    }
  }

  if (!actions_(verb, noun)->isRunning())
  {
    LogTrace << "Running action..." << std::endl;
    actions_(verb, noun)->run();
  }
}

void HmiLogic::error()
{
  LogError << "Error state" << std::endl;
}

void HmiLogic::registerAction(Action *action)
{
  actions_.registerAction(action);
}

void HmiLogic::resetHmi()
{
  data_.nVerb = -1;
  data_.nNoun = -1;
  data_.nR1 = -1;
  data_.nR2 = -1;
  data_.nR3 = -1;

  data_.nProgMode = DRAW_OFF;
  data_.nVerbMode = DRAW_OFF;
  data_.nNounMode = DRAW_OFF;

  data_.nR1Mode = DRAW_OFF;
  data_.nR2Mode = DRAW_OFF;
  data_.nR3Mode = DRAW_OFF;

  data_.lProgMode = DRAW_OFF;
  data_.lVerbMode = DRAW_OFF;
  data_.lNounMode = DRAW_OFF;

  data_.lCompActyMode = DRAW_OFF;

  data_.lR1Mode = DRAW_OFF;
  data_.lR2Mode = DRAW_OFF;
  data_.lR3Mode = DRAW_OFF;
}

void HmiLogic::verbHmi()
{
  resetHmi();
  data_.lVerbMode = DRAW_ON;
}

void HmiLogic::nounHmi()
{
  verbHmi();
  data_.lNounMode = DRAW_ON;
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
