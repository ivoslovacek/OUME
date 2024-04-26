#pragma once

#include <qimage.h>
#include <qlabel.h>
#include <qobject.h>
#include <qpainter.h>
#include <qt/QtCore/qcoreevent.h>
#include <qtimer.h>
#include <qtmetamacros.h>
#include <qwindowdefs.h>

#include <memory>

#include "events.hpp"
#include "ffmpeg/player.hpp"

namespace OUMP {
/**
 * @brief QWidget that actually displays the media content.
 */
class MediaFrame : public QLabel {
    Q_OBJECT
   private slots:
    void handleNewFile(QString t_filename);
    void onTick();

   private:
    std::shared_ptr<EventsHub> m_events;
    QTimer* m_timer;

    std::shared_ptr<MediaDecoder> m_decoder;
    std::shared_ptr<FrameData> m_current_frame;
    bool m_playing;

   public:
    MediaFrame(std::shared_ptr<EventsHub> t_events);
    ~MediaFrame() = default;
};
}  // namespace OUMP
