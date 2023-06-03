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
  if (keyEvents_.size() == 1)
  {
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

    case MODE_RUN:
      run(data);
      break;

    case MODE_ERROR:
      error(data);
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
    run(data);
  }

  /* Publish the data */
  // data = data_;
  // std::memcpy(data, &data_, sizeof(HmiData));
}

void HmiLogic::idle(HmiData &data)
{
  if (keyEvents_[0].getEvent() == E_KEY_VERB)
  {
    LogTrace << "VERB" << std::endl;

    data.rSetVerbLabel(DRAW_ON);

    status_.transit(MODE_V_INPUT);
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

    /**
     * @todo put needNoun in a try to capture unknown
     * actions
     */

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
    status_.transit(MODE_IDLE);
    break;

  default:
    LogWarning << "Not available operation" << std::endl;
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

  case E_KEY_NOUN:
  case E_KEY_CLR:
    data.rSetnNoun(UNASIGNED, DRAW_OFF);
    break;

  case E_KEY_RSET:
    data.resetRightLcdData();
    status_.transit(MODE_IDLE);
    break;

  default:
    LogWarning << "Not available operation" << std::endl;
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

    switch (eventKey)
    {
    case E_KEY_RSET:
      actions_(data)->stop();
      data.resetRightLcdData();
      status_.transit(MODE_IDLE);
      return;

    default:
      break;
    }
  }

  if (!actions_(data)->isRunning())
  {
    LogTrace << "Running action..." << std::endl;
    actions_(data)->run();
  }
}

void HmiLogic::error(HmiData &data)
{
  LogError << "Error state" << std::endl;
}

void HmiLogic::registerAction(Action *action)
{
  actions_.registerAction(action);
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
