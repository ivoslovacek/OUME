#include "media.hpp"

#include <qimage.h>
#include <qpainter.h>
#include <qsizepolicy.h>
#include <qtimer.h>
#include <qwidget.h>
#include <qwindowdefs.h>

#include <memory>

#include "ffmpeg/player.hpp"

namespace OUMP {
MediaView::MediaView(std::shared_ptr<EventsHub> t_events)
    : QWidget(),
      m_events(t_events),
      m_painter(std::make_shared<QPainter>(this)),
      m_frame(std::make_shared<MediaFrame>(t_events)){};

MediaFrame::MediaFrame(std::shared_ptr<EventsHub> t_events)
    : QWidget(),
      m_events(t_events),
      m_timer(new QTimer(this)),
      m_decoder(std::shared_ptr<MediaDecoder>()),
      m_current_frame(std::shared_ptr<FrameData>()),
      m_playing(false) {
    connect(this->m_events.get(), &EventsHub::changedFileName, this,
            &MediaFrame::handleNewFile);

    connect(this->m_timer, &QTimer::timeout, this, &MediaFrame::onTick);
    this->m_timer->start(41);
    this->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
};

void MediaFrame::handleNewFile(QString t_filename) {
    this->m_decoder = std::make_shared<MediaDecoder>(t_filename.toStdString());
    this->m_playing = true;
}

void MediaFrame::onTick() {
    if (!this->m_playing) {
        return;
    }

    auto l_tmp = this->m_current_frame;
    this->m_current_frame = this->m_decoder->nextFrame();
    this->repaint();
}

void MediaFrame::paintEvent(QPaintEvent *t_event) {
    if (this->m_current_frame != nullptr) {
        QPainter l_painter(this);
        l_painter.drawImage(QRect(0, 0, 1280, 720),
                            this->m_current_frame->getImage());
    }
    QWidget::paintEvent(t_event);
}
}  // namespace OUMP
