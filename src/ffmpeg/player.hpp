#pragma once

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavcodec/codec.h>
#include <libavcodec/packet.h>
#include <libavformat/avformat.h>
#include <libavutil/frame.h>
#include <libavutil/opt.h>
#include <libswresample/swresample.h>
}
#include <qimage.h>

#include <cstdint>
#include <future>
#include <memory>
#include <mutex>
#include <queue>
#include <string>

namespace OUMP {
class FrameData;
class SamplesData;
/**
 * @brief FrameData Comparator
 */
struct FrameDataComparator {
   public:
    bool operator()(const std::shared_ptr<FrameData> &a,
                    const std::shared_ptr<FrameData> &b);
};

/**
 * @brief SamplesData Comparator
 */
struct SamplesDataComparator {
   public:
    bool operator()(const std::shared_ptr<SamplesData> &a,
                    const std::shared_ptr<SamplesData> &b);
};

/**
 * @brief A class for decoding media.
 */
class MediaDecoder {
   private:
    AVFormatContext *m_format_context;
    std::string m_filename;

    const AVCodec *m_video_codec;
    AVCodecContext *m_video_context;
    AVPacket *m_video_packet;
    AVFrame *m_video_frame;
    std::optional<int> m_video_stream_index = 0;

    const AVCodec *m_audio_codec;
    AVCodecContext *m_audio_context;
    SwrContext *m_audio_swr_context;
    std::optional<int> m_audio_stream_index = 0;

    std::priority_queue<std::shared_ptr<FrameData>,
                        std::vector<std::shared_ptr<FrameData>>,
                        FrameDataComparator>
        m_frame_queue;
    std::priority_queue<std::shared_ptr<SamplesData>,
                        std::vector<std::shared_ptr<SamplesData>>,
                        SamplesDataComparator>
        m_samples_queue;

    std::shared_ptr<FrameData> m_frame_buffer;
    std::mutex m_queue_mutex;

    std::optional<std::future<void>> m_decoding_future;
    bool m_decoding;
    bool m_finished = false;
    std::mutex m_controls_mutex;

    /**
     * @brief Main decoding loop.
     */
    void decodingLoop();

   public:
    /**
     * @brief Constructor for MediaDecoder.
     *
     * This constructor takes the filename of the media file to be decoded
     * as a QString argument.
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
     *
     * @return int Integer indicating if a frame was decoded succesfullu (see
     * avcodec_receive_frame).
     */
    int decodeNextVideoPacket();

    /**
     * @brief Starts the decoding of the media;
     */
    void startDecoding();

    /**
     * @brief Stops the decoding of the media;
     */
    void stopDecoding();

    /**
     * @brief Returns a bool indicating if there is no more media to be
     * presented.
     */
    inline bool GetMediaEOFStatus() {
        this->m_controls_mutex.lock();
        this->m_queue_mutex.lock();

        bool l_result = (this->m_frame_queue.size() == 0 && this->m_finished)
                            ? true
                            : false;

        this->m_controls_mutex.unlock();
        this->m_queue_mutex.unlock();

        return l_result;
    }

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

class SamplesData {
   private:
    uint8_t *m_data;
    size_t m_size;

    /**
     * @brief int64_t containing the presentation time stamp of the samples.
     */
    int64_t m_pts;

   public:
    SamplesData(AVFrame *t_frame);
    ~SamplesData();

    inline size_t getSize() const { return this->m_size; }

    inline const uint8_t *getData() const { return this->m_data; }

    /**
     * @brief Getter function for the frame presentation timestamp.
     *
     * @return int64_t containing the presentation timestamp.
     */
    inline int64_t getPts() const { return this->m_pts; }
};
}  // namespace OUMP
