#include "controls.hpp"

#include <qpushbutton.h>
#include <qwidget.h>

namespace OUMP {

ControlsLayout::ControlsLayout(std::shared_ptr<EventsHub> t_events)
    : QHBoxLayout(), m_play_pause(new PlayPauseButton()), m_events(t_events) {
    this->addWidget(this->m_play_pause);
    connect(this->m_play_pause, &PlayPauseButton::changedPlayingState,
            t_events.get(), &EventsHub::changePlayingState);
}

// TODO: Fix initial state
PlayPauseButton::PlayPauseButton() : QPushButton("Pause") {
    this->m_playing = true;
    this->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
}

void PlayPauseButton::mousePressEvent(QMouseEvent* t_event) {
    this->buttonPressed();
    QPushButton::mousePressEvent(t_event);
}

void PlayPauseButton::buttonPressed() {
    if (this->m_playing) {
        this->setText("Play");
        this->m_playing = false;
    } else {
        this->setText("Pause");
        this->m_playing = true;
    }

    emit(this->changedPlayingState(this->m_playing));
}
}  // namespace OUMP
