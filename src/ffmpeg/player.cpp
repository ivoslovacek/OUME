#include "player.hpp"

#include "ffmpeg/exceptions.hpp"

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavcodec/codec.h>
#include <libavcodec/codec_par.h>
#include <libavcodec/packet.h>
#include <libavformat/avformat.h>
#include <libavutil/dict.h>
#include <libavutil/error.h>
#include <libavutil/frame.h>
#include <libavutil/imgutils.h>
#include <libavutil/log.h>
#include <libavutil/mem.h>
#include <libswscale/swscale.h>
}

#include <qimage.h>

#include <cerrno>
#include <iostream>
#include <memory>
#include <new>
#include <optional>
#include <ostream>

namespace OUMP {
#ifdef __DEBUG__
void log_callback(void *, int, const char *fmt, va_list vl) {
    // Print the log message to the console
    vprintf(fmt, vl);
}
#endif
bool FrameDataComparator::operator()(const std::shared_ptr<FrameData> &a,
                                     const std::shared_ptr<FrameData> &b) {
    return a->getPts() > b->getPts();
}

MediaDecoder::MediaDecoder(QString t_filename)
    : MediaDecoder(t_filename.toStdString()) {}

// TODO: create a mechanism for unified audio decoding
MediaDecoder::MediaDecoder(std::string t_filename)
    : m_filename(t_filename),
      m_video_codec(nullptr),
      m_video_context(nullptr),
      m_video_packet(nullptr),
      m_video_frame(nullptr),
      m_video_stream_index(std::nullopt),
      m_audio_codec(nullptr),
      m_audio_context(nullptr) {
#ifdef __DEBUG__
    av_log_set_level(AV_LOG_TRACE);
    av_log_set_callback(log_callback);
#endif

    // allocate memory for media
    this->m_format_context = avformat_alloc_context();
    if (this->m_format_context == nullptr) {
        std::cerr << "Couldn't allocate memory for media context" << std::endl;
        throw std::bad_alloc();
    }

    // try to open media file
    bool l_mediaOpened = !static_cast<bool>(-avformat_open_input(
        &this->m_format_context, this->m_filename.c_str(), nullptr, nullptr));
    if (!l_mediaOpened) {
        throw OUMP::UnreadableFileException();
    }

    // try to find the best video codec
    int l_stream_number =
        av_find_best_stream(this->m_format_context, AVMEDIA_TYPE_VIDEO, -1, -1,
                            &this->m_video_codec, 0);

    if (l_stream_number != AVERROR_STREAM_NOT_FOUND ||
        l_stream_number != AVERROR_DECODER_NOT_FOUND ||
        this->m_video_codec == nullptr) {
        this->m_video_stream_index = l_stream_number;

        // allocate memory for video context
        this->m_video_context = avcodec_alloc_context3(this->m_video_codec);
        if (this->m_video_context == nullptr) {
            std::cerr << "Couldn't allocate memory for video context"
                      << std::endl;
            throw std::bad_alloc();
        }

        // create video context from video parameters and initialize it
        avcodec_parameters_to_context(
            this->m_video_context,
            this->m_format_context->streams[this->m_video_stream_index.value()]
                ->codecpar);
        avcodec_open2(this->m_video_context, this->m_video_codec, nullptr);

        // alocate video packet
        this->m_video_packet = av_packet_alloc();
        if (this->m_video_packet == nullptr) {
            std::cerr << "Couldn't allocate memory for video packet"
                      << std::endl;
            throw std::bad_alloc();
        }

        // alocate video frame
        this->m_video_frame = av_frame_alloc();
        if (this->m_video_frame == nullptr) {
            std::cerr << "Couldn't allocate memory for video frame"
                      << std::endl;
            throw std::bad_alloc();
        }
    }
#ifdef __DEBUG__
    else {
        std::cerr
            << "Video stream or a decoder for a video stream couldn't be found"
            << std::endl;
    }
#endif
}

MediaDecoder::~MediaDecoder() {
    avformat_close_input(&this->m_format_context);
    avformat_free_context(this->m_format_context);

    if (this->m_video_context != nullptr) {
        avcodec_free_context(&this->m_video_context);
    }

    if (this->m_video_packet != nullptr) {
        av_packet_free(&this->m_video_packet);
    }

    if (this->m_video_frame != nullptr) {
        av_frame_free(&this->m_video_frame);
    }

    if (this->m_audio_context != nullptr) {
        avcodec_free_context(&this->m_audio_context);
    }
}

void MediaDecoder::decodeNextVideoPacket() {
    int l_response =
        av_read_frame(this->m_format_context, this->m_video_packet);

    if (this->m_video_packet->stream_index != this->m_video_stream_index) {
        av_packet_unref(this->m_video_packet);
        return;
    }

    if (l_response >= 0) {
        l_response =
            avcodec_send_packet(this->m_video_context, this->m_video_packet);
        if (l_response < 0) {
#ifdef __DEBUG__
            std::cerr << "Error while sending a packet to the decoder"
                      << std::endl;
#endif
            return;
        }

        l_response =
            avcodec_receive_frame(this->m_video_context, this->m_video_frame);

        if (l_response < 0) {
#ifdef __DEBUG__
            std::cerr << "Decoding error: " << l_response << std::endl;
#endif
        } else {
            while (l_response >= 0) {
                if (l_response == AVERROR(EAGAIN) ||
                    l_response == AVERROR_EOF) {
                    std::cerr << "EAGAIN OR EOF" << std::endl;
                    break;
                } else if (l_response < 0) {
                    std::cerr
                        << "Error while receiving a frame from the decoder"
                        << std::endl;
                    return;
                }

                this->m_frame_queue.push(
                    std::make_shared<FrameData>(this->m_video_frame));

                l_response = avcodec_receive_frame(this->m_video_context,
                                                   this->m_video_frame);
            }
        }

        av_packet_unref(this->m_video_packet);
    } else {
#ifdef __DEBUG__
        std::cerr << "Error while getting the next video frame" << std::endl;
#endif
        return;
    }
}

std::shared_ptr<FrameData> MediaDecoder::nextFrame() {
    if (this->m_frame_queue.size() <= 1) {
        this->decodeNextVideoPacket();
    }

    if (this->m_frame_queue.size() <= 1) {
        this->decodeNextVideoPacket();
    }

    if (this->m_frame_queue.size() == 0) {
        return std::shared_ptr<FrameData>();
    }

    auto l_next_frame = this->m_frame_queue.top();
    this->m_frame_queue.pop();
    return l_next_frame;
}

FrameData::FrameData(AVFrame *t_frame) : m_pts(t_frame->pts) {
    SwsContext *scaler_ctx =
        sws_getContext(t_frame->width, t_frame->height,
                       static_cast<AVPixelFormat>(t_frame->format),
                       t_frame->width, t_frame->height, AV_PIX_FMT_RGB32,
                       SWS_BICUBIC, nullptr, nullptr, nullptr);

    if (scaler_ctx == nullptr) {
        std::cerr << "Couldn't allocate memory for sws ctx" << std::endl;
        throw std::bad_alloc();
    }

    AVFrame *l_frame = av_frame_alloc();
    if (l_frame == nullptr) {
        std::cerr << "Couldn't allocate memory for a new AVFrame" << std::endl;
        throw std::bad_alloc();
    }
    size_t l_frame_size = av_image_get_buffer_size(
        AV_PIX_FMT_RGB32, t_frame->width, t_frame->height, 1);
    auto l_alloc_result =
        av_image_alloc(l_frame->data, l_frame->linesize, t_frame->width,
                       t_frame->height, AV_PIX_FMT_RGB32, 1);
    if (l_alloc_result < 0) {
        std::cerr << "Couldn't alloacte memory for a image" << std::endl;
        throw std::bad_alloc();
    }
    l_frame->format = AV_PIX_FMT_RGB32;

    sws_scale(scaler_ctx, t_frame->data, t_frame->linesize, 0, t_frame->height,
              l_frame->data, l_frame->linesize);

    this->m_image =
        QImage(t_frame->width, t_frame->height, QImage::Format_RGB32);
    std::copy(l_frame->data[0], l_frame->data[0] + l_frame_size,
              this->m_image.bits());

    av_freep(&l_frame->data[0]);
    av_frame_free(&l_frame);
    sws_freeContext(scaler_ctx);
}
}  // namespace OUMP
