#include "layout.hpp"

#include <qboxlayout.h>
#include <qslider.h>
#include <qwidget.h>

#include "player/seekslider.hpp"

namespace OUMP {
PlayerLayout::PlayerLayout(std::shared_ptr<EventsHub> t_events)
    : QVBoxLayout(),
      m_events(t_events),
      m_controls(new ControlsLayout(t_events)),
      m_seek_slider(new SeekSliderLayout(t_events)),
      m_media(new MediaFrame(t_events)) {
    this->addWidget(this->m_media, 2);
    this->addLayout(this->m_seek_slider);
    this->addLayout(this->m_controls);
};
}  // namespace OUMP
