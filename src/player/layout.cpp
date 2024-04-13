#include "layout.hpp"

#include <qboxlayout.h>
#include <qwidget.h>

namespace OUMP {
PlayerLayout::PlayerLayout(std::shared_ptr<EventsHub> t_events)
    : QVBoxLayout(),
      m_events(t_events),
      m_controls(new ControlsLayout()),
      m_media(new MediaFrame(t_events)) {
    this->addWidget(this->m_media, 2);
    this->addLayout(this->m_controls);
};
}  // namespace OUMP
