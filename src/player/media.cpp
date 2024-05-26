#include "media.hpp"

#include <qimage.h>
#include <qlabel.h>
#include <qmessagebox.h>
#include <qnamespace.h>
#include <qpainter.h>
#include <qsizepolicy.h>
#include <qtimer.h>
#include <qwidget.h>
#include <qwindowdefs.h>

#include <cstdint>
#include <memory>

#include "events.hpp"
#include "ffmpeg/exceptions.hpp"
#include "ffmpeg/player.hpp"

namespace OUMP {
MediaFrame::MediaFrame(std::shared_ptr<EventsHub> t_events)
    : QLabel(),
      m_events(t_events),
      m_decoder(std::shared_ptr<MediaDecoder>()),
      m_current_frame(std::shared_ptr<FrameData>()),
      m_playing(false) {
    connect(this->m_events.get(), &EventsHub::changedFileName, this,
            &MediaFrame::handleNewFile);
    connect(this->m_events.get(), &EventsHub::changedPlayingState, this,
            &MediaFrame::changePlayingState);
    connect(this, &MediaFrame::changedPlayingState, this->m_events.get(),
            &EventsHub::changePlayingState);
    connect(this->m_events.get(), &EventsHub::changedVolume, this,
            &MediaFrame::changeVolume);
    connect(this->m_events.get(), &EventsHub::changedCurrentSliderTimepoint,
            this, &MediaFrame::changeMediaTimepoint);
    connect(this, &MediaFrame::changedMediaTimepoint, this->m_events.get(),
            &EventsHub::changeCurrentMediaTimepoint);
    connect(this, &MediaFrame::changedMediaEndTimepoint, this->m_events.get(),
            &EventsHub::changedMediaEndTimepoint);

    this->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    this->setAlignment(Qt::AlignCenter);
    this->setStyleSheet("background-color: black; color: white;");
};

void MediaFrame::handleNewFile(QString t_filename) {
    if (t_filename.isEmpty()) {
        return;
    }

    try {
        this->m_decoder =
            std::make_shared<MediaDecoder>(t_filename.toStdString());
        this->m_decoder->startDecoding();
        this->m_playing = true;
        emit this->changedMediaEndTimepoint(this->m_decoder->getDuration());
        emit this->changedMediaTimepoint(0);
        emit this->changedPlayingState(this->m_playing);
    } catch (const UnreadableFileException& e) {
        QMessageBox l_error(QMessageBox::Critical, "Error", e.what(),
                            QMessageBox::Ok);
        l_error.exec();
    }
}

void MediaFrame::changePlayingState(bool t_state) {
    if (this->m_decoder == nullptr) {
        return;
    }

    this->m_playing = t_state;
    this->m_playing ? this->m_decoder->startDecoding()
                    : this->m_decoder->stopDecoding();
}

void MediaFrame::changeVolume(int t_volume) {
    if (this->m_decoder != nullptr) {
        this->m_decoder->setVolume(static_cast<float>(t_volume / 100.0));
    }
}

void MediaFrame::changeMediaTimepoint(int64_t t_timepoint) {
    if (this->m_decoder == nullptr) {
        return;
    }

    // This is really fucking stupid, but the simplest way to deal with
    // key-frames
    this->m_decoder->seekMedia(t_timepoint);
    this->m_decoder->seekMedia(t_timepoint);
}

void MediaFrame::paintEvent(QPaintEvent* t_event) {
    if (this->m_playing) {
        auto l_tmp = this->m_decoder->nextFrame();
        if (l_tmp != nullptr) {
            if (this->m_decoder->GetMediaEOFStatus()) {
                emit this->changedPlayingState(false);
            }
            this->m_current_frame = l_tmp;

            auto l_pixmap =
                QPixmap::fromImage(this->m_current_frame->getImage())
                    .scaled(this->width(), this->height(), Qt::KeepAspectRatio,
                            Qt::SmoothTransformation);
            this->setPixmap(l_pixmap);

            emit this->changedMediaTimepoint(this->m_decoder->getCurrentPts());
        }
    }

    QLabel::paintEvent(t_event);
}

void MediaFrame::resizeEvent(QResizeEvent* t_event) {
    auto l_pixmap =
        this->pixmap().scaled(this->width(), this->height(),
                              Qt::KeepAspectRatio, Qt::SmoothTransformation);
    this->setPixmap(l_pixmap);

    QLabel::resizeEvent(t_event);
}

}  // namespace OUMP
