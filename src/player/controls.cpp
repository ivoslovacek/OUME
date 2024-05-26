#include "controls.hpp"

#include <qnamespace.h>
#include <qpushbutton.h>
#include <qslider.h>
#include <qwidget.h>

#include "events.hpp"

namespace OUMP {

ControlsLayout::ControlsLayout(std::shared_ptr<EventsHub> t_events)
    : QHBoxLayout(),
      m_play_pause(new PlayPauseButton()),
      m_volume_slider(new VolumeSlider()),
      m_events(t_events) {
    this->addStretch();
    this->addWidget(this->m_play_pause);
    this->addStretch();
    this->addWidget(this->m_volume_slider);

    connect(this->m_play_pause, &PlayPauseButton::changedPlayingState,
            t_events.get(), &EventsHub::changePlayingState);

    connect(this->m_events.get(), &EventsHub::changedPlayingState,
            this->m_play_pause, &PlayPauseButton::changePlayingState);

    connect(this->m_volume_slider, &VolumeSlider::valueChanged, t_events.get(),
            &EventsHub::changeVolume);
}

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

VolumeSlider::VolumeSlider(QWidget* t_parent) : QSlider(t_parent) {
    this->setMinimum(0);
    this->setMaximum(150);
    this->setSliderPosition(100);
    this->setTickPosition(QSlider::TicksBelow);
    this->setOrientation(Qt::Horizontal);
    this->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    setValue(50);
}
}  // namespace OUMP
