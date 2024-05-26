#pragma once

#include <qboxlayout.h>
#include <qwidget.h>

#include <memory>

#include "events.hpp"
#include "player/controls.hpp"
#include "player/media.hpp"
#include "player/seekslider.hpp"

namespace OUMP {
/**
 * @brief Media player layout.
 */
class PlayerLayout : public QVBoxLayout {
   private:
    std::shared_ptr<EventsHub> m_events;
    ControlsLayout* m_controls;
    SeekSliderLayout* m_seek_slider;
    MediaFrame* m_media;

   public:
    PlayerLayout(std::shared_ptr<EventsHub> t_events);
    ~PlayerLayout() = default;
};
}  // namespace OUMP
