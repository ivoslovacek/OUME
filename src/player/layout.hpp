#pragma once

#include <qboxlayout.h>
#include <qwidget.h>

#include <memory>

#include "events.hpp"
#include "player/controls.hpp"
#include "player/media.hpp"

namespace OUMP {
class PlayerLayout : public QVBoxLayout {
   private:
    std::shared_ptr<EventsHub> m_events;
    ControlsLayout* m_controls;
    MediaFrame* m_media;

   public:
    PlayerLayout(std::shared_ptr<EventsHub> t_events);
    ~PlayerLayout() = default;
};
}  // namespace OUMP
