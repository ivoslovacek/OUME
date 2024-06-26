#pragma once

#include <qimage.h>
#include <qlabel.h>
#include <qobject.h>
#include <qpainter.h>
#include <qtimer.h>
#include <qtmetamacros.h>
#include <qwidget.h>
#include <qwindowdefs.h>

#include <cstdint>
#include <memory>

#include "events.hpp"
#include "ffmpeg/player.hpp"

namespace OUMP {
/**
 * @brief QWidget that actually displays the media content.
 */
class MediaFrame : public QLabel {
    Q_OBJECT
   signals:
    void changedPlayingState(bool);
    void changedMediaTimepoint(int64_t);
    void changedMediaEndTimepoint(int64_t);

   private slots:
    void handleNewFile(QString t_filename);
    void changePlayingState(bool);
    void changeVolume(int);
    void changeMediaTimepoint(int64_t);

   private:
    std::shared_ptr<EventsHub> m_events;

    std::shared_ptr<MediaDecoder> m_decoder;
    std::shared_ptr<FrameData> m_current_frame;
    bool m_playing;

    void paintEvent(QPaintEvent* t_event) override;

   public:
    MediaFrame(std::shared_ptr<EventsHub> t_events);
    ~MediaFrame() = default;

    void resizeEvent(QResizeEvent* t_event) override;
};
}  // namespace OUMP
