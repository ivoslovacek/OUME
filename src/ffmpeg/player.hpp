#pragma once

#include <cstdint>
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
/**
 * @brief FrameDataComparator
 */
struct FrameDataComparator {
   public:
    bool operator()(const std::shared_ptr<FrameData> &a,
                    const std::shared_ptr<FrameData> &b);
};

/**
 * @brief A class for decoding media.
 */
class MediaDecoder {
   private:
    /**
     * @brief Media format context.
     */
    AVFormatContext *m_format_context;
    /**
     * @brief Media file name.
     */
    std::string m_filename;

    /**
     * @brief Video codec for the media.
     */
    const AVCodec *m_video_codec;
    /**
     * @brief video context for the media.
     */
    AVCodecContext *m_video_context;
    /**
     * @brief Video packet for the media.
     */
    AVPacket *m_video_packet;
    /**
     * @brief Video frame for the media.
     */
    AVFrame *m_video_frame;
    /**
     * @brief Index of the video stream to decode.
     */
    std::optional<int> m_video_stream_index = 0;

    /**
     * @brief Audio codec for the media.
     */
    const AVCodec *m_audio_codec;
    /**
     * @brief Audio context for the media.
     */
    AVCodecContext *m_audio_context;

    /**
     * @brief Video frame queue for the media.
     */
    std::priority_queue<std::shared_ptr<FrameData>,
                        std::vector<std::shared_ptr<FrameData>>,
                        FrameDataComparator>
        m_frame_queue;

   public:
    /**
     * @brief Constructor for MediaDecoder.
     *
     * This constructor takes the filename of the media file to be decoded as a
     * QString argument.
     *
     * @param t_filename The filename of the media file.
     */
    MediaDecoder(QString t_filename);
    /**
     * @brief Constructor for MediaDecoder.
     *
     * This constructor takes the filename of the media file to be decoded as a
     * std::string argument.
     *
     * @param t_filename The filename of the media file.
     */
    MediaDecoder(std::string t_filename);

    /**
     * @brief Destructor for MediaDecoder.
     */
    ~MediaDecoder();

    /**
     * @brief Decodes the next video packet from the media file.
     *
     * This function decodes the next video packet from the media file and
     * updates internal state.
     */
    void decodeNextVideoPacket();
    /**
     * @brief Retrieves the next decoded frame from the decoder.
     *
     * This function retrieves the next decoded video frame from the internal
     * queue.
     *
     * @return A shared pointer to a FrameData object containing the decoded
     * frame data.
     */
    std::shared_ptr<FrameData> nextFrame();
};

/**
 * @brief A class representing a decoded video frame.
 */
class FrameData {
   private:
    /**
     * @brief QImage containing the decoded image data.
     */
    QImage m_image;

    /**
     * @brief int64_t containing the presentation time stamp of the frame.
     */
    int64_t m_pts;

   public:
    /**
     * @brief Constructor for FrameData.
     *
     * This constructor takes an AVFrame pointer as input and extracts the image
     * data in RGB32 format to populate the QImage member.
     *
     * @param t_frame Pointer to the AVFrame containing the decoded image data.
     */
    FrameData(AVFrame *t_frame);

    /**
     * @brief Destructor for FrameData.
     */
    ~FrameData() = default;

    /**
     * @brief Getter function for the image data.
     *
     * This function returns a const reference to the QImage member, allowing
     * access to the decoded image data.
     *
     * @return A const reference to the QImage containing the decoded image.
     */
    inline const QImage &getImage() const { return this->m_image; }

    /**
     * @brief Getter function for the frame presentation timestamp.
     *
     * @return int64_t containing the presentation timestamp.
     */
    inline int64_t getPts() const { return this->m_pts; }
};
}  // namespace OUMP
