#pragma once

#include <qboxlayout.h>
#include <qpushbutton.h>
#include <qslider.h>
#include <qtmetamacros.h>
#include <qwidget.h>

#include <memory>

#include "events.hpp"

namespace OUMP {
class PlayPauseButton;
class VolumeSlider;

/**
 * @brief Media controls layout.
 */
class ControlsLayout : public QHBoxLayout {
   private:
    PlayPauseButton* m_play_pause;
    VolumeSlider* m_volume_slider;
    std::shared_ptr<EventsHub> m_events;

   public:
    ControlsLayout(std::shared_ptr<EventsHub> t_events);
    ~ControlsLayout() = default;
};

/**
 * @brief Play/Pause button.
 */
class PlayPauseButton : public QPushButton {
    Q_OBJECT
   signals:
    void changedPlayingState(bool);

   private:
    bool m_playing = false;

    void buttonPressed();
    void mousePressEvent(QMouseEvent* t_event) override;

   public slots:
    void changePlayingState(bool);

   public:
    PlayPauseButton();
    ~PlayPauseButton() = default;

    inline bool getState() const { return this->m_playing; };
};

class VolumeSlider : public QSlider {
   public:
    VolumeSlider(QWidget* t_parent = nullptr);
    ~VolumeSlider() = default;
};
}  // namespace OUMP
