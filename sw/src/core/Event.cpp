#include "Event.h"


Event::Event() {

}

Event::Event(const EventId_t event) {

    record(event);
}

Event::~Event() {

}

void Event::record(const EventId_t event) {

    eventId_ = event;
    timestamp_ = std::time(nullptr);
}

EventId_t Event::getEvent() {
    return eventId_;
}

std::time_t Event::getTimestamp() {
    return timestamp_;
}