#include "events.hpp"

namespace OUMP {
EventsHub::EventsHub() {}

void EventsHub::changeFileName(QString t_filename) {
    this->m_filename = t_filename;
    emit this->changedFileName(t_filename);
}

void EventsHub::changePlayingState(bool t_state) {
    emit this->changedPlayingState(t_state);
}
}  // namespace OUMP
