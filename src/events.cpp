#include "events.hpp"

#include <iostream>

namespace OUMP {
EventsHub::EventsHub() {}

void EventsHub::changeFileName(QString t_filename) {
    this->m_filename = t_filename;
    emit this->changedFileName(t_filename);
}

void EventsHub::changePlayingState(bool t_state) {
    emit this->changedPlayingState(t_state);
}

void EventsHub::changeVolume(int t_volume) {
    emit this->changedVolume(t_volume);
}

void EventsHub::changeMediaEndTimepoint(int64_t t_timepoint) {
    emit this->changedMediaEndTimepoint(t_timepoint);
}

void EventsHub::changeCurrentMediaTimepoint(int64_t t_timepoint) {
    emit this->changedCurrentMediaTimepoint(t_timepoint);
}
void EventsHub::changeCurrentSliderTimepoint(int64_t t_timepoint) {
    emit this->changedCurrentSliderTimepoint(t_timepoint);
}
}  // namespace OUMP
