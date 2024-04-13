#pragma once

#include <qboxlayout.h>
#include <qpushbutton.h>
#include <qwidget.h>

#include <memory>

namespace OUMP {
class PlayPauseButton;

class ControlsLayout : public QHBoxLayout {
   private:
    PlayPauseButton* m_play_pause;

   public:
    ControlsLayout();
    ~ControlsLayout() = default;
};

class PlayPauseButton : public QPushButton {
   private:
    bool m_playing = false;
    void mousePressEvent(QMouseEvent* event) override;

   public:
    PlayPauseButton();
    ~PlayPauseButton() = default;

    inline bool getState() const { return this->m_playing; };
};
}  // namespace OUMP
