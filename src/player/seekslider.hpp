#pragma once

#include <qboxlayout.h>
#include <qlabel.h>
#include <qslider.h>
#include <qtmetamacros.h>
#include <qwidget.h>

#include <QMouseEvent>
#include <chrono>
#include <cstdint>
#include <iostream>
#include <memory>

#include "events.hpp"

namespace OUMP {
class TimePointDisplay;
class SeekSlider;

class SeekSliderLayout : public QHBoxLayout {
   private:
    std::shared_ptr<EventsHub> m_events;
    TimePointDisplay* m_start_time;
    SeekSlider* m_seek_slider;
    TimePointDisplay* m_end_time;

   public:
    SeekSliderLayout(std::shared_ptr<EventsHub> t_events);
    ~SeekSliderLayout() = default;
};

class TimePointDisplay : public QLabel {
    Q_OBJECT
   public:
    TimePointDisplay(QWidget* t_parent = nullptr);
    ~TimePointDisplay() = default;

    void setTime(int64_t t_timepoint);
   public slots:
    void changeTimePoint(int64_t);
};

class SeekSlider : public QSlider {
    Q_OBJECT
   signals:
    void changedSeekValue(int64_t);

   private:
    bool m_user_seek = true;

   public:
    SeekSlider(QWidget* t_parent = nullptr);
    ~SeekSlider() = default;

    void mousePressEvent(QMouseEvent* event) override {
        auto l_tmp = static_cast<int>(
            this->minimum() +
            ((this->maximum() - this->minimum()) * event->position().x()) /
                width());
        emit this->changedSeekValue(
            std::chrono::duration_cast<std::chrono::microseconds>(
                std::chrono::seconds(l_tmp))
                .count());

        QSlider::mousePressEvent(event);
    }

   public slots:
    void changeSliderPosition(int64_t);
    void changeEndTimepoint(int64_t);
    void checkUserValueChange(int);
};
}  // namespace OUMP
