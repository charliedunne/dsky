#include "HmiLogic.h"

#include "Logger.h"


/**
 * @brief Function to resolve the falue of the key event
 *
 * @return key value
 */
int key(EventId_t);


HmiLogic::HmiLogic() {

  status_ = HSM_IDLE;
}

HmiLogic::~HmiLogic() {
}

void HmiLogic::parseKeyEvents(std::vector<Event> events) {

  keyEvents_ = events;
}

void HmiLogic::updateHmiData(HmiData * data) {


  if (keyEvents_.size() == 1) {

    switch (status_) {

    case HSM_IDLE:
      idle();
      break;

    case HSM_V_INPUT:
      verbInput();
      break;

    case HSM_N_INPUT:
      nounInput();
      break;

    case HSM_RUN:
      run();
      break;

    default:
      LogError << "Unknown Status Machine" << std::endl;
      break;
    }
  }
  else if (keyEvents_.size() > 1) {

    /**
     * @todo What to do for multiple events at once
     */

    LogWarning << "@TODO Handle more than one event at one" << std::endl;
  }

  /* Publish the data */
  std::memcpy(data, &data_, sizeof(HmiData));
}

void HmiLogic::idle() {

  if (keyEvents_[0].getEvent() == E_KEY_VERB) {

    LogTrace << "VERB" << std::endl;

    data_.nProgMode = DRAW_OFF;
    data_.nVerbMode = DRAW_OFF;
    data_.nNounMode = DRAW_OFF;

    data_.nR1Mode = DRAW_OFF;
    data_.nR2Mode = DRAW_OFF;
    data_.nR3Mode = DRAW_OFF;

    data_.lProgMode = DRAW_ON;
    data_.lVerbMode = DRAW_ON;
    data_.lNounMode = DRAW_ON;

    data_.lCompActyMode = DRAW_OFF;

    data_.lR1Mode = DRAW_ON;
    data_.lR2Mode = DRAW_ON;
    data_.lR3Mode = DRAW_ON;
    
    status_ = HSM_V_INPUT;
  }
}

void HmiLogic::verbInput() {

  /* Copy the Event key received */
  EventId_t eventKey = keyEvents_[0].getEvent();

  /* Set-up the labels for Verb and disable Noun */
  data_.lVerbMode = DRAW_ON;
  data_.lNounMode = DRAW_OFF;

  switch(eventKey) {
    
  case E_KEY_ENTR:
  case E_KEY_NOUN:

    /**
     * @TODO check if the verb can be executed by itself
     */
    data_.lNounMode = DRAW_ON;
    status_ = HSM_N_INPUT;
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

    if (data_.nVerbMode == DRAW_OFF) {
      
      data_.nVerbMode = DRAW_ON;
      data_.nVerb = key(eventKey);
    } 
    else {
      data_.nVerb = data_.nVerb*10 + key(eventKey);
    }
    break;

  case E_KEY_VERB:
  case E_KEY_CLR:
    data_.nVerbMode = DRAW_OFF;
    data_.nVerb = 0;
    break;

  case E_KEY_RSET:
    /** @todo clear all */
    status_ = HSM_IDLE;
    break;


  default:
    LogWarning << "Not available operation" << std::endl;
    break;
  }

}

void HmiLogic::nounInput() {

  /* Copy the Event key received */
  EventId_t eventKey = keyEvents_[0].getEvent();

  /* Set-up the labels for Verb and disable Noun */
  data_.lVerbMode = DRAW_ON;
  data_.lNounMode = DRAW_ON;

  switch(eventKey) {
    
  case E_KEY_ENTR:

    status_ = HSM_RUN;
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
    
    if (data_.nNounMode == DRAW_OFF) {
      
      data_.nNounMode = DRAW_ON;
      data_.nNoun = key(eventKey);
    } 
    else {
      data_.nNoun = data_.nNoun*10 + key(eventKey);
    }

    break;

  case E_KEY_VERB:
    data_.nVerb = 0;
    data_.nNoun = 0;

    data_.nVerbMode = DRAW_OFF;
    data_.nNounMode = DRAW_OFF;
    data_.lVerbMode = DRAW_ON;
    data_.lNounMode = DRAW_OFF;
    status_ = HSM_V_INPUT;

  case E_KEY_NOUN:
  case E_KEY_CLR:
    data_.nNounMode = DRAW_OFF;
    data_.nNoun = 0;
    break;

  case E_KEY_RSET:
    /** @todo clear all */
    status_ = HSM_IDLE;
    break;

  default:
    LogWarning << "Not available operation" << std::endl;
    break;
  }

}

void HmiLogic::run() {

  LogTrace << "Running program..." << std::endl;
}

int key(EventId_t id) {

  /* Return value */
  int value = 0;

  switch (id) {

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
