#include "media.hpp"

#include <qimage.h>
#include <qlabel.h>
#include <qpainter.h>
#include <qsizepolicy.h>
#include <qtimer.h>
#include <qwidget.h>
#include <qwindowdefs.h>

#include <memory>

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

    connect(this->m_timer, &QTimer::timeout, this, &MediaFrame::onTick);
    this->m_timer->start(41);
    this->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    this->setScaledContents(true);
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
    this->setPixmap(QPixmap::fromImage(this->m_current_frame->getImage()));
}
}  // namespace OUMP
