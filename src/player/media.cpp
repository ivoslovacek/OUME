#include "media.hpp"

#include <qimage.h>
#include <qlabel.h>
#include <qnamespace.h>
#include <qpainter.h>
#include <qsizepolicy.h>
#include <qtimer.h>
#include <qwidget.h>
#include <qwindowdefs.h>

#include <iostream>
#include <memory>
#include <ostream>

#include "events.hpp"
#include "ffmpeg/player.hpp"

namespace OUMP {
MediaFrame::MediaFrame(std::shared_ptr<EventsHub> t_events)
    : QLabel(),
      m_events(t_events),
      m_timer(new QTimer(this)),
      m_decoder(std::shared_ptr<MediaDecoder>()),
      m_current_frame(std::shared_ptr<FrameData>()),
      m_playing(false) {
    connect(this->m_events.get(), &EventsHub::changedFileName, this,
            &MediaFrame::handleNewFile);
    connect(this->m_events.get(), &EventsHub::changedPlayingState, this,
            &MediaFrame::changePlayingState);

    connect(this->m_timer, &QTimer::timeout, this, &MediaFrame::onTick);
    this->m_timer->start(41);
    this->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    this->setAlignment(Qt::AlignCenter);
};

void MediaFrame::handleNewFile(QString t_filename) {
    this->m_decoder = std::make_shared<MediaDecoder>(t_filename.toStdString());
    this->m_decoder->startDecoding();
    this->m_playing = true;
}

void MediaFrame::changePlayingState(bool t_state) {
    this->m_playing = t_state;

    this->m_playing ? this->m_decoder->startDecoding()
                    : this->m_decoder->stopDecoding();
}

void MediaFrame::onTick() {
    if (!this->m_playing) {
        return;
    }

    auto l_tmp = this->m_current_frame;
    this->m_current_frame = this->m_decoder->nextFrame();

    if (this->m_current_frame == nullptr) {
        return;
    }

    auto l_pixmap = QPixmap::fromImage(this->m_current_frame->getImage())
                        .scaled(this->width(), this->height(),
                                Qt::KeepAspectRatio, Qt::SmoothTransformation);

    this->setPixmap(l_pixmap);
}

void MediaFrame::resizeEvent(QResizeEvent* t_event) {
    auto l_pixmap =
        this->pixmap().scaled(this->width(), this->height(),
                              Qt::KeepAspectRatio, Qt::SmoothTransformation);
    this->setPixmap(l_pixmap);

    QLabel::resizeEvent(t_event);
}

}  // namespace OUMP
