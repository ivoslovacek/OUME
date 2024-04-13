#pragma once

#include <qimage.h>
#include <qobject.h>
#include <qt/QtCore/qcoreevent.h>
#include <qtimer.h>
#include <qtmetamacros.h>
#include <qwidget.h>
#include <qwindowdefs.h>
#include <qpainter.h>

#include <memory>

#include "events.hpp"
#include "ffmpeg/player.hpp"

namespace OUMP {
class MediaFrame;

class MediaView : public QWidget {
   private:
    std::shared_ptr<EventsHub> m_events;
    std::shared_ptr<QPainter> m_painter;
    std::shared_ptr<MediaFrame> m_frame;

   public:
    MediaView(std::shared_ptr<EventsHub> t_events);
    ~MediaView() = default;
};

class MediaFrame : public QWidget {
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

    protected:
        void paintEvent(QPaintEvent *t_event) override;

   public:
    MediaFrame(std::shared_ptr<EventsHub> t_events);
    ~MediaFrame() = default;
};
}  // namespace OUMP
