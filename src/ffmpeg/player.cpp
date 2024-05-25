#include "player.hpp"

#include <qaudioformat.h>

#include <stdexcept>

#include "ffmpeg/exceptions.hpp"

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavcodec/codec.h>
#include <libavcodec/codec_par.h>
#include <libavcodec/packet.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libavutil/channel_layout.h>
#include <libavutil/dict.h>
#include <libavutil/error.h>
#include <libavutil/frame.h>
#include <libavutil/imgutils.h>
#include <libavutil/log.h>
#include <libavutil/mem.h>
#include <libavutil/samplefmt.h>
#include <libswresample/swresample.h>
#include <libswscale/swscale.h>
}

#include <pulse/error.h>
#include <pulse/simple.h>
#include <qimage.h>

#include <cerrno>
#include <cstdint>
#include <future>
#include <iostream>
#include <memory>
#include <mutex>
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

bool SamplesDataComparator::operator()(const std::shared_ptr<SamplesData> &a,
                                       const std::shared_ptr<SamplesData> &b) {
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
      m_audio_context(nullptr),
      m_frame_queue(std::priority_queue<std::shared_ptr<FrameData>,
                                        std::vector<std::shared_ptr<FrameData>>,
                                        FrameDataComparator>()),
      m_frame_buffer(std::shared_ptr<FrameData>()),
      m_queue_mutex(std::mutex()),
      m_decoding_future(std::nullopt),
      m_decoding(false),
      m_controls_mutex(std::mutex()) {
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
    }
#ifdef __DEBUG__
    else {
        std::cerr
            << "Video stream or a decoder for a video stream couldn't be found"
            << std::endl;
    }
#endif

    // try to find the best audio codec
    l_stream_number =
        av_find_best_stream(this->m_format_context, AVMEDIA_TYPE_AUDIO, -1, -1,
                            &this->m_audio_codec, 0);

    if (l_stream_number != AVERROR_STREAM_NOT_FOUND ||
        l_stream_number != AVERROR_DECODER_NOT_FOUND ||
        this->m_audio_codec == nullptr) {
        this->m_audio_stream_index = l_stream_number;

        // allocate memory for video context
        this->m_audio_context = avcodec_alloc_context3(this->m_audio_codec);
        if (this->m_audio_context == nullptr) {
            std::cerr << "Couldn't allocate memory for audio context"
                      << std::endl;
            throw std::bad_alloc();
        }

        // create audio context from audio parameters and initialize it
        avcodec_parameters_to_context(
            this->m_audio_context,
            this->m_format_context->streams[this->m_audio_stream_index.value()]
                ->codecpar);
        avcodec_open2(this->m_audio_context, this->m_audio_codec, nullptr);
    }
#ifdef __DEBUG__
    else {
        std::cerr
            << "Audio stream or a decoder for an audio stream couldn't be found"
            << std::endl;
    }
#endif

    // allocate video packet
    this->m_video_packet = av_packet_alloc();
    if (this->m_video_packet == nullptr) {
        std::cerr << "Couldn't allocate memory for video packet" << std::endl;
        throw std::bad_alloc();
    }

    // allocate video frame
    this->m_video_frame = av_frame_alloc();
    if (this->m_video_frame == nullptr) {
        std::cerr << "Couldn't allocate memory for video frame" << std::endl;
        throw std::bad_alloc();
    }
}

MediaDecoder::~MediaDecoder() {
    this->m_decoding = false;

    if (this->m_decoding_future.has_value()) {
        this->m_decoding_future->wait();
    }

    avformat_close_input(&this->m_format_context);
    avformat_free_context(this->m_format_context);

    if (this->m_video_context != nullptr) {
        avcodec_free_context(&this->m_video_context);
    }

    if (this->m_video_packet != nullptr) {
        av_packet_free(&this->m_video_packet);
    }

    if (this->m_audio_context != nullptr) {
        avcodec_free_context(&this->m_audio_context);
    }

    if (this->m_video_frame != nullptr) {
        av_frame_free(&this->m_video_frame);
    }

    if (this->m_audio_context != nullptr) {
        avcodec_free_context(&this->m_audio_context);
    }
}

int MediaDecoder::decodeNextVideoPacket() {
    std::cerr << "tady";
    int l_response =
        av_read_frame(this->m_format_context, this->m_video_packet);

    if (this->m_video_packet->stream_index == this->m_video_stream_index) {
        if (l_response >= 0) {
            l_response = avcodec_send_packet(this->m_video_context,
                                             this->m_video_packet);
            if (l_response < 0) {
#ifdef __DEBUG__
                std::cerr << "Error while sending a packet to the decoder"
                          << std::endl;
#endif
                return l_response;
            }

            l_response = avcodec_receive_frame(this->m_video_context,
                                               this->m_video_frame);

            if (l_response < 0) {
#ifdef __DEBUG__
                std::cerr << "Decoding error: " << l_response << std::endl;
#endif
            } else {
                while (l_response >= 0) {
                    if (l_response == AVERROR(EAGAIN) ||
                        l_response == AVERROR_EOF) {
                        std::cerr << "EAGAIN OR EOF" << std::endl;
                        return l_response;
                    } else if (l_response < 0) {
                        std::cerr
                            << "Error while receiving a frame from the decoder"
                            << std::endl;
                        return l_response;
                    }

                    this->m_queue_mutex.lock();
                    this->m_frame_queue.push(
                        std::make_shared<FrameData>(this->m_video_frame));
                    this->m_queue_mutex.unlock();

                    l_response = avcodec_receive_frame(this->m_video_context,
                                                       this->m_video_frame);
                }
            }

            av_packet_unref(this->m_video_packet);
        } else {
#ifdef __DEBUG__
            std::cerr << "Error while getting the next video frame"
                      << std::endl;
#endif
        }

        return l_response;
    } else if (this->m_video_packet->stream_index ==
               this->m_audio_stream_index) {
        if (l_response >= 0) {
            l_response = avcodec_send_packet(this->m_audio_context,
                                             this->m_video_packet);
            if (l_response < 0) {
#ifdef __DEBUG__
                std::cerr << "Error while sending a packet to the decoder"
                          << std::endl;
#endif
                return l_response;
            }

            l_response = avcodec_receive_frame(this->m_audio_context,
                                               this->m_video_frame);

            if (l_response < 0) {
#ifdef __DEBUG__
                std::cerr << "Decoding error: " << l_response << std::endl;
#endif
            } else {
                while (l_response >= 0) {
                    if (l_response == AVERROR(EAGAIN) ||
                        l_response == AVERROR_EOF) {
                        std::cerr << "EAGAIN OR EOF" << std::endl;
                        return l_response;
                    } else if (l_response < 0) {
                        std::cerr << "Error while receiving a frame from "
                                     "the decoder"
                                  << std::endl;
                        return l_response;
                    }

                    this->m_samples_queue.push(
                        std::make_shared<SamplesData>(this->m_video_frame));

                    l_response = avcodec_receive_frame(this->m_audio_context,
                                                       this->m_video_frame);
                }
            }

            av_packet_unref(this->m_video_packet);
        } else {
#ifdef __DEBUG__
            std::cerr << "Error while getting the next video frame"
                      << std::endl;
#endif
        }

        return l_response;
    } else {
        av_packet_unref(this->m_video_packet);
        return -11;
    }
}

// TODO: move switching of the currently displayed frame here
void MediaDecoder::decodingLoop() {
    this->m_controls_mutex.lock();

    auto l_sample_rate =
        static_cast<uint32_t>(this->m_audio_context->sample_rate);
    static const pa_sample_spec l_sample_spec = {PA_SAMPLE_S16LE, l_sample_rate,
                                                 2};

    int error;
    pa_simple *l_pulse_audio =
        pa_simple_new(nullptr, "OUMP", PA_STREAM_PLAYBACK, nullptr, "playback",
                      &l_sample_spec, nullptr, nullptr, &error);
    if (l_pulse_audio == nullptr) {
#ifdef __DEBUG__
        std::cerr << "failed to initialize audio playback: "
                  << pa_strerror(error) << std::endl;
#endif
        throw std::bad_alloc();
    }

    while (this->m_decoding) {
        this->m_controls_mutex.unlock();

        while (this->m_frame_queue.size() <= 5 ||
               this->m_samples_queue.size() <= 5) {
            auto l_result = this->decodeNextVideoPacket();

            if (l_result == AVERROR_EOF) {
#ifdef __DEBUG__
                std::cerr << "end of video" << std::endl;
#endif
                this->m_controls_mutex.lock();
                this->m_decoding = false;
                this->m_finished = true;
                this->m_controls_mutex.unlock();
                break;
            }
        }

        while (!this->m_samples_queue.empty()) {
            auto l_sample = this->m_samples_queue.top();
            this->m_samples_queue.pop();
            std::cerr << l_sample->getSize() << std::endl;

            if (pa_simple_write(l_pulse_audio, l_sample->getData(),
                                l_sample->getSize(), &error) < 0) {
#ifdef __DEBUG__
                std::cerr << "pa_simple_write() failed: " << pa_strerror(error)
                          << std::endl;
#endif
            }
        }

        this->m_controls_mutex.lock();
    }

    if (pa_simple_drain(l_pulse_audio, &error) < 0) {
        std::cerr << "pa_simple_drain() failed: " << pa_strerror(error)
                  << std::endl;
        throw std::runtime_error("pa drained");
    }

    pa_simple_free(l_pulse_audio);

    this->m_controls_mutex.unlock();
}

void MediaDecoder::startDecoding() {
    if (this->m_decoding) {
        return;
    } else {
        this->m_decoding = true;
    }

    this->m_decoding_future = std::async(
        std::launch::async, std::bind(&MediaDecoder::decodingLoop, this));

    this->m_controls_mutex.unlock();
}

void MediaDecoder::stopDecoding() {
    this->m_controls_mutex.lock();
    this->m_decoding = false;
    this->m_controls_mutex.unlock();

    this->m_decoding_future->wait();
    this->m_decoding_future = std::nullopt;
}

std::shared_ptr<FrameData> MediaDecoder::nextFrame() {
    if (this->m_queue_mutex.try_lock()) {
        if (this->m_frame_queue.size() != 0) {
            this->m_frame_buffer = this->m_frame_queue.top();
            this->m_frame_queue.pop();
        }

        this->m_queue_mutex.unlock();
    }

    return this->m_frame_buffer;
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
SamplesData::SamplesData(AVFrame *t_frame) : m_pts(t_frame->pts) {
    SwrContext *l_swr_ctx = nullptr;

    AVChannelLayout l_output_layout;
    av_channel_layout_from_mask(&l_output_layout,
                                static_cast<uint64_t>(AV_CH_LAYOUT_STEREO));

    int l_result = swr_alloc_set_opts2(
        &l_swr_ctx, &l_output_layout, AV_SAMPLE_FMT_S16,
        static_cast<uint32_t>(t_frame->sample_rate), &t_frame->ch_layout,
        static_cast<AVSampleFormat>(t_frame->format), t_frame->sample_rate, 0,
        nullptr);

    if (l_result < 0) {
        throw std::bad_alloc();
    }

    if (swr_init(l_swr_ctx) < 0) {
        swr_free(&l_swr_ctx);
    }

    int l_converted_samples_count =
        swr_get_out_samples(l_swr_ctx, t_frame->nb_samples);

    int l_tmp = av_samples_get_buffer_size(nullptr, l_output_layout.nb_channels,
                                           l_converted_samples_count,
                                           AV_SAMPLE_FMT_S16, 0);

    if (l_tmp < 0) {
        throw std::bad_alloc();
    }

    this->m_size = l_tmp;

    this->m_data = new uint8_t[this->m_size];

    swr_convert(l_swr_ctx, &this->m_data, l_converted_samples_count,
                const_cast<const uint8_t **>(t_frame->data),
                t_frame->nb_samples);

    swr_free(&l_swr_ctx);
}

SamplesData::~SamplesData() { delete[] this->m_data; }
}  // namespace OUMP
