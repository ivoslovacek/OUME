#include "controls.hpp"

#include <qpushbutton.h>
#include <qwidget.h>

#include "events.hpp"

namespace OUMP {

ControlsLayout::ControlsLayout(std::shared_ptr<EventsHub> t_events)
    : QHBoxLayout(), m_play_pause(new PlayPauseButton()), m_events(t_events) {
    this->addWidget(this->m_play_pause);
    connect(this->m_play_pause, &PlayPauseButton::changedPlayingState,
            t_events.get(), &EventsHub::changePlayingState);

    connect(this->m_events.get(), &EventsHub::changedPlayingState,
            this->m_play_pause, &PlayPauseButton::changePlayingState);
}

// TODO: Fix initial state
PlayPauseButton::PlayPauseButton() : QPushButton("Play"), m_playing(false) {
    this->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
}

void PlayPauseButton::mousePressEvent(QMouseEvent* t_event) {
    this->buttonPressed();
    QPushButton::mousePressEvent(t_event);
}

void PlayPauseButton::buttonPressed() {
    emit(this->changedPlayingState(!this->m_playing));
}
void PlayPauseButton::changePlayingState(bool t_state) {
    this->m_playing = t_state;

    if (this->m_playing) {
        this->setText("Pause");
    } else {
        this->setText("Play");
    }
}
}  // namespace OUMP
