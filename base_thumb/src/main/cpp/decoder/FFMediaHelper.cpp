//
// Created by 廖海龙 on 2024/7/24.
//

#include "FFMediaHelper.hpp"


FFMediaHelper::FFMediaHelper() : logger("FFMediaHelper"), uri()
{

}

FFMediaHelper::~FFMediaHelper()
{

}

void FFMediaHelper::setDataSource(std::string _uri)
{
    logger.i("setDataSource uri: %s", uri.c_str());
    this->uri = _uri;
}


int FFMediaHelper::getVideoRotation(AVFormatContext *fmt, AVStream *stream)
{

    AVDictionaryEntry *tag = av_dict_get(stream->metadata, "rotate", nullptr, 0);
    if (tag)
    {
        return std::stoi(tag->value);
    }

    const AVCodec *codec = avcodec_find_decoder(stream->codecpar->codec_id);
    if (!codec)
    {
        logger.i("codec not found");
        return 0;
    }

    AVCodecContext *codec_ctx = avcodec_alloc_context3(codec);
    if (!codec_ctx)
    {
        logger.i("codec context not found");
        return 0;
    }
    // 把解码配置参数传给解码器上下文
    if (avcodec_parameters_to_context(codec_ctx, stream->codecpar) < 0)
    {
        return 0;
    }
    // 打开解码器，不然解析不出数据
    if (avcodec_open2(codec_ctx, codec, nullptr) < 0)
    {
        return 0;
    }

    int rotation = 0;

    AVPacket *packet = av_packet_alloc();
    AVFrame *frame = av_frame_alloc();

    // 抓出轨道数据
    while (av_read_frame(fmt, packet) >= 0)
    {
        if (packet->stream_index == stream->index)
        {
            if (packet->flags & AV_PKT_FLAG_KEY)
            {
                break;
            }
        }
    }

    // 送解
    while (true)
    {
        if (avcodec_send_packet(codec_ctx, packet) >= 0)
        {
            break;
        }
    }


    // flush the decoder
    avcodec_send_packet(codec_ctx, nullptr);

    // 取出解码数据
    while (true)
    {
        if (avcodec_receive_frame(codec_ctx, frame) >= 0)
        {
            // 判断是否存在旋转角度
            double theta = 0.0;
            int32_t *displayMatrix = nullptr;
            AVFrameSideData *sd = av_frame_get_side_data(frame, AV_FRAME_DATA_DISPLAYMATRIX);
            if (sd)
            {
                displayMatrix = (int32_t *) sd->data;
                if (!displayMatrix)
                {
                    const AVPacketSideData *psd = av_packet_side_data_get(
                            stream->codecpar->coded_side_data,
                            stream->codecpar->nb_coded_side_data,
                            AV_PKT_DATA_DISPLAYMATRIX);
                    if (psd)
                        displayMatrix = (int32_t *) psd->data;
                }
                theta = getRotation(displayMatrix);
            }
            rotation = static_cast<int>(theta);
            break;
        }
    }

    av_packet_free(&packet);
    av_frame_free(&frame);
    avcodec_free_context(&codec_ctx);
    return rotation;
}

double FFMediaHelper::getRotation(const int32_t *displaymatrix)
{
    double theta = 0;
    if (displaymatrix)
        theta = -round(av_display_rotation_get(displaymatrix));

    theta -= 360 * floor(theta / 360 + 0.9 / 360);

    if (fabs(theta - 90 * round(theta / 90)) > 2)
        logger.i("Odd rotation angle.\n"
                 "If you want to help, upload a sample "
                 "of this file to https://streams.videolan.org/upload/ "
                 "and contact the ffmpeg-devel mailing list. (ffmpeg-devel@ffmpeg.org)");

    return theta;
}

std::shared_ptr<FFMetaData> FFMediaHelper::getMetaData()
{
    std::shared_ptr<FFMetaData> metaData = std::make_shared<FFMetaData>();


    AVFormatContext *fmt_ctx = nullptr;
    AVDictionaryEntry *tag = nullptr;

    // 获取数据
    if (avformat_open_input(&fmt_ctx, uri.c_str(), nullptr, nullptr) < 0)
    {
        return metaData;
    }

    // 获取封装信息
    while ((tag = av_dict_get(fmt_ctx->metadata, "", tag, AV_DICT_IGNORE_SUFFIX)))
    {
        metaData->tag.insert(std::make_pair(tag->key, tag->value));
        logger.i("tag: [%s:%s]", tag->key, tag->value);
    }

    // 拆封装，取出轨道信息
    int ret = avformat_find_stream_info(fmt_ctx, nullptr);
    if (ret >= 0)
    {
        for (int i = 0; i < fmt_ctx->nb_streams; ++i)
        {
            AVStream *stream = fmt_ctx->streams[i];
            AVCodecContext *codec_ctx = avcodec_alloc_context3(nullptr);
            // 依附轨道信息，后面才能取出相关内容
            avcodec_parameters_to_context(codec_ctx, stream->codecpar);
            // 编码类型
            const char *codec_name = avcodec_get_name(codec_ctx->codec_id);
            // 时长
            int64_t durationUs = av_rescale_q(stream->duration, stream->time_base,
                                              (AVRational) {1, 1000000});

            if (stream->codecpar->codec_type == AVMEDIA_TYPE_AUDIO)
            {
                auto audio = std::make_shared<AudioMetaData>();

                audio->mime = codec_name;
                audio->sampleRate = codec_ctx->sample_rate;
                audio->channelCount = codec_ctx->ch_layout.nb_channels;
                audio->bitsPerSample = codec_ctx->bits_per_coded_sample;
                audio->durationUs = durationUs;

                metaData->audioTrackers.push_back(audio);


            } else if (stream->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
            {
                auto video = std::make_shared<VideoMetaData>();

                double fps = av_q2d(stream->avg_frame_rate);

                video->mime = codec_name;
                video->width = stream->codecpar->width;
                video->height = stream->codecpar->height;
                video->fps = static_cast<int>(fps);
                video->bitrate = codec_ctx->bit_rate;
                video->rotation = getVideoRotation(fmt_ctx, stream);
                video->durationUs = durationUs;

                metaData->videoTrackers.push_back(video);
            }

            avcodec_free_context(&codec_ctx);
        }
    }


    avformat_close_input(&fmt_ctx);
    return metaData;
}