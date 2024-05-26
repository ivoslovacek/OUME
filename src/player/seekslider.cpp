#include "seekslider.hpp"

#include <qlabel.h>
#include <qnamespace.h>
#include <qslider.h>

#include <chrono>
#include <cstdint>
#include <iomanip>
#include <sstream>

#include "events.hpp"

namespace OUMP {
SeekSliderLayout::SeekSliderLayout(std::shared_ptr<EventsHub> t_events)
    : QHBoxLayout(),
      m_start_time(new TimePointDisplay()),
      m_seek_slider(new SeekSlider()),
      m_end_time(new TimePointDisplay()) {
    this->addWidget(this->m_start_time);
    this->m_start_time->setTime(0);
    this->addWidget(this->m_seek_slider);
    this->addWidget(this->m_end_time);
    this->m_end_time->setTime(0);

    connect(t_events.get(), &EventsHub::changedMediaEndTimepoint,
            this->m_end_time, &TimePointDisplay::changeTimePoint);
    connect(t_events.get(), &EventsHub::changedMediaEndTimepoint,
            this->m_seek_slider, &SeekSlider::changeEndTimepoint);
    connect(t_events.get(), &EventsHub::changedCurrentMediaTimepoint,
            this->m_seek_slider, &SeekSlider::changeSliderPosition);
    connect(t_events.get(), &EventsHub::changedCurrentMediaTimepoint,
            this->m_start_time, &TimePointDisplay::changeTimePoint);

    connect(this->m_seek_slider, &SeekSlider::changedSeekValue, t_events.get(),
            &EventsHub::changeCurrentSliderTimepoint);
}

TimePointDisplay::TimePointDisplay(QWidget* t_parent) : QLabel(t_parent) {
    this->setTime(0);
}
void TimePointDisplay::setTime(int64_t t_timepoint) {
    auto l_timepoint = std::chrono::microseconds(t_timepoint);

    std::stringstream l_ss;
    l_ss << std::setfill('0') << std::setw(2)
         << std::chrono::duration_cast<std::chrono::seconds>(l_timepoint)
                    .count() /
                60;
    l_ss << ":";
    l_ss << std::setfill('0') << std::setw(2)
         << std::chrono::duration_cast<std::chrono::seconds>(l_timepoint)
                    .count() %
                60;

    this->setText(l_ss.str().c_str());
}

void TimePointDisplay::changeTimePoint(int64_t t_timepoint) {
    this->setTime(t_timepoint);
}

SeekSlider::SeekSlider(QWidget* t_parent) : QSlider(t_parent) {
    this->setOrientation(Qt::Horizontal);
    this->setMinimum(0);
    this->setMaximum(0);
    this->setTickInterval(1);
    this->setSliderPosition(0);
    this->setSizePolicy(QSizePolicy::MinimumExpanding,
                        QSizePolicy::MinimumExpanding);

    connect(this, &SeekSlider::valueChanged, this,
            &SeekSlider::checkUserValueChange);
}

void SeekSlider::changeSliderPosition(int64_t t_timepoint) {
    auto l_tmp =
        static_cast<int>(std::chrono::duration_cast<std::chrono::seconds>(
                             std::chrono::microseconds(t_timepoint))
                             .count());
    this->m_user_seek = false;
    this->setSliderPosition(l_tmp);
}

void SeekSlider::changeEndTimepoint(int64_t t_timepoint) {
    auto l_tmp =
        static_cast<int>(std::chrono::duration_cast<std::chrono::seconds>(
                             std::chrono::microseconds(t_timepoint))
                             .count());
    this->setMaximum(l_tmp);
}

void SeekSlider::checkUserValueChange(int t_timepoint) {
    if (this->m_user_seek) {
        emit this->changedSeekValue(static_cast<int64_t>(t_timepoint) * 1000 *
                                    1000);
    } else {
        this->m_user_seek = true;
    }
}
}  // namespace OUMP
