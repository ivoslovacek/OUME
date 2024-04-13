#pragma once

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavcodec/codec.h>
#include <libavcodec/packet.h>
#include <libavformat/avformat.h>
#include <libavutil/frame.h>
}

#include <qimage.h>

#include <memory>
#include <queue>
#include <string>

namespace OUMP {
class FrameData;

class MediaDecoder {
   private:
    AVFormatContext *m_format_context;
    std::string m_filename;

    const AVCodec *m_video_codec;
    AVCodecContext *m_video_context;
    AVPacket *m_video_packet;
    AVFrame *m_video_frame;
    int m_video_stream_index = 0;

    const AVCodec *m_audio_codec;
    AVCodecContext *m_audio_context;

    int64_t m_current_pts;
    std::optional<int64_t> m_pts_diff;
    std::queue<std::shared_ptr<FrameData>> m_frame_queue;

   public:
    MediaDecoder(QString t_filename);
    MediaDecoder(std::string t_filename);
    ~MediaDecoder();

    void decodeNextVideoPacket();
    std::shared_ptr<FrameData> nextFrame();
};

class FrameData {
   private:
    QImage m_image;

   public:
    FrameData(AVFrame *t_frame);
    ~FrameData() = default;

    inline const QImage getImage() const { return this->m_image; };
};
}  // namespace OUMP
