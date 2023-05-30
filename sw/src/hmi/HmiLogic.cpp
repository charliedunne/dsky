#include "HmiLogic.h"

#include "Logger.h"

HmiLogic::HmiLogic() {
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

    /* Publish the data */
    memcpy(data, &data_, sizeof(HmiData));

  }
  else {

    /**
     * @todo What to do for multiple events at once
     */

    LogWarning << "@TODO Handle more than one event at one" << std::endl;
  }
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
  }
}

void HmiLogic::verbInput() {
}

void HmiLogic::nounInput() {
}

void HmiLogic::run() {
}
