#include "controls.hpp"

#include <qpushbutton.h>

namespace OUMP {

ControlsLayout::ControlsLayout()
    : QHBoxLayout(), m_play_pause(new PlayPauseButton()) {
    this->addWidget(this->m_play_pause);
}

PlayPauseButton::PlayPauseButton() : QPushButton("Play") {
    this->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
}

void PlayPauseButton::mousePressEvent(QMouseEvent *t_event) {
    if (this->m_playing) {
        this->setText("Play");
        this->m_playing = false;
    } else {
        this->setText("Pause");
        this->m_playing = true;
    }
    QPushButton::mousePressEvent(t_event);
}
}  // namespace OUMP
