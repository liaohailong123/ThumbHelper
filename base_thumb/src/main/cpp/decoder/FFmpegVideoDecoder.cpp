//
// Created by liaohailong on 2024/6/16.
//

#include "FFmpegVideoDecoder.hpp"


FFmpegVideoDecoder::FFmpegVideoDecoder() : logger("FFmpegVideoDecoder"), jniCaller(nullptr),
                                           uri(""), surface(nullptr),
                                           fmt_ctx(nullptr), video_dec_ctx(nullptr),
                                           video_stream_idx(-1),
                                           groups(),
                                           decodeThread(nullptr), decodeHandler(nullptr),
                                           decodedFrames(),
                                           canvas(nullptr), shouldAutoRender(true),
                                           decodeComplete(false)
{
    logger.i("FFmpegVideoDecoder::FFmpegVideoDecoder()");
}

FFmpegVideoDecoder::~FFmpegVideoDecoder()
{
    avcodec_free_context(&video_dec_ctx);
    avformat_close_input(&fmt_ctx);

    // 回收分组数据资源
    groups.clear();

    // jni回调删除
    delete jniCaller;

    // 释放Handler
    if (decodeThread != nullptr)
    {
        delete decodeThread;
        decodeHandler = nullptr;
    }

    // 释放画布资源，内部含有EGLContext和EGLSurface
    if (canvas != nullptr)
    {
        delete canvas;
        canvas = nullptr;
    }
    logger.i("FFmpegVideoDecoder::~FFmpegVideoDecoder()");

}

void FFmpegVideoDecoder::setJniInstance(jobject instance)
{
    delete jniCaller;
    jniCaller = new FFmpegVideoDecoderJniCaller(instance);
}

int FFmpegVideoDecoder::setDataSource(const char *_uri)
{
    uri = _uri;

    if (avformat_open_input(&fmt_ctx, _uri, nullptr, nullptr))
    {
        return -1;
    }

    if (avformat_find_stream_info(fmt_ctx, nullptr) < 0)
    {
        logger.i("Cannot find stream information");
        return -1;
    }

    if (open_codec_context(&video_stream_idx, &video_dec_ctx, fmt_ctx, AVMEDIA_TYPE_VIDEO) >= 0)
    {
        return 0;
    }


    return -1;
}

void FFmpegVideoDecoder::setSurface(ANativeWindow *_surface)
{
    int width = video_dec_ctx->width;
    int height = video_dec_ctx->height;
    // 给OES纹理设置尺寸
    ANativeWindow_setBuffersGeometry(_surface, width, height,
                                     AHARDWAREBUFFER_FORMAT_R8G8B8A8_UNORM);
    surface = _surface;

    // 初始化 YUVCanvas
    if (canvas == nullptr)
    {
        canvas = new YUVCanvas(surface, width, height);
    }
    // 自建EGL环境，把这个 surface 制作成 EGLSurface
    // 创建sampler2D纹理
    // ffmpeg解码出来的YUV设置到 sampler2D 上
    // 绘制图元 -> eglSwapBuffers

}

int64_t FFmpegVideoDecoder::getPreviousIFrameUs(int64_t timestampsUs)
{
    if (video_dec_ctx == nullptr)
    {
        return -1;
    }

    AVRational time_base = fmt_ctx->streams[video_stream_idx]->time_base;
    // us to pts
    int64_t pts_unit = av_rescale_q(timestampsUs, (AVRational) {1, 1000000}, time_base);


    int ret = av_seek_frame(fmt_ctx, video_stream_idx, pts_unit, AVSEEK_FLAG_BACKWARD);

    if (ret < 0)
    {
        return -1;
    }

    // 刷新输入文件的解码器上下文
    avcodec_flush_buffers(video_dec_ctx);

    // 开始读取数据
    int64_t pts = -1;
    AVPacket _packet;
    while (av_read_frame(fmt_ctx, &_packet) >= 0)
    {
        if (_packet.stream_index == video_stream_idx)
        {
            if (_packet.flags & AV_PKT_FLAG_KEY)
            {
                // pts to us
                int64_t pts_us = av_rescale_q(_packet.pts, time_base, (AVRational) {1, 1000000});
                pts = pts_us;
                break;
            }
        }
    }

    return
            pts;
}

int FFmpegVideoDecoder::open_codec_context(int *stream_idx, AVCodecContext **dec_ctx,
                                           AVFormatContext *_fmt_ctx, enum AVMediaType type)
{
    int ret, stream_index;
    AVStream *st;
    const AVCodec *dec = NULL;


    ret = av_find_best_stream(_fmt_ctx, type, -1, -1, NULL, 0);

    if (ret < 0)
    {
        logger.i("Could not find %s stream in input file '%s'", av_get_media_type_string(type),
                 uri.c_str());
        return ret;
    } else
    {
        stream_index = ret;
        st = fmt_ctx->streams[stream_index];

        /* find decoder for the stream */
        dec = avcodec_find_decoder(st->codecpar->codec_id);
        if (dec == NULL)
        {
            logger.i("Failed to find %s codec", av_get_media_type_string(type));
            return -1;
        }

        /* Allocate a codec context for the decoder */
        *dec_ctx = avcodec_alloc_context3(dec);
        if (*dec_ctx == NULL)
        {
            logger.i("Failed to allocate the %s codec context", av_get_media_type_string(type));
            return -1;
        }

        /* Copy codec parameters from input stream to output codec context */
        if ((ret = avcodec_parameters_to_context(*dec_ctx, st->codecpar)) < 0)
        {
            logger.i("Failed to copy %s codec parameters to decoder context",
                     av_get_media_type_string(type));
            return ret;
        }


        if ((ret = avcodec_open2(*dec_ctx, dec, NULL)) < 0)
        {
            logger.i("Failed to open %s codec", av_get_media_type_string(type));
            return ret;
        }

        *stream_idx = stream_index;
    }

    return 0;
}

void FFmpegVideoDecoder::start(int64_t *frameUs, int count, bool exactly)
{
    // 回收分组数据资源
    groups.clear();

    // 对解码任务进行分组
    if (exactly)
    {
        // 精准模式
        std::unordered_map<long, std::shared_ptr<FFDecoder::FrameGroupInfo>> map;
        for (int i = 0; i < count; ++i)
        {
            int64_t seekToUs = frameUs[i];
            // 找出画面所属的I帧，进行分组，后续统一跟着I帧解码，提高效率
            int64_t iFrame = getPreviousIFrameUs(seekToUs);

            auto iter = map.find(iFrame);
            std::shared_ptr<FFDecoder::FrameGroupInfo> group;
            if (iter == map.end())
            {
                // 新的I帧，创建分组
                group = std::make_shared<FFDecoder::FrameGroupInfo>();
                group->timestampsUs = iFrame;
                map.insert(std::make_pair(iFrame, group));
                groups.push_back(group);
            } else
            {
                group = iter->second;
            }
            group->frames.push_back(seekToUs);
        }

    } else
    {
        // 非精准模式
        for (int i = 0; i < count; ++i)
        {
            int64_t seekToUs = frameUs[i];
            int64_t iFrame = getPreviousIFrameUs(seekToUs);

            auto group = std::make_shared<FFDecoder::FrameGroupInfo>();
            group->timestampsUs = iFrame;
            group->frames.push_back(iFrame);
            groups.push_back(group);
        }

    }

    // 开始分组解码
    if (decodeThread == nullptr)
    {
        decodeThread = new NHandlerThread("ffmpeg-video-decoder-handler-thread");
        decodeThread->start([](NHandler *handler, void *args) -> void {
            FFmpegVideoDecoder *decoder = reinterpret_cast<FFmpegVideoDecoder *>(args);
            decoder->decodeHandler = handler;
            // 开始解码任务
            decoder->decodeAll();

        }, this);
    } else
    {
        if (decodeHandler != nullptr)
        {
            // restart
            decodeHandler->post([](void *args) -> void {
                FFmpegVideoDecoder *decoder = reinterpret_cast<FFmpegVideoDecoder *>(args);
                // 开始解码任务
                decoder->decodeAll();

            }, this);
        }
    }

}


void FFmpegVideoDecoder::decodeAll()
{
    decodeComplete = false;
    shouldAutoRender = true;
    while (!groups.empty())
    {
        std::shared_ptr<FFDecoder::FrameGroupInfo> group = groups.front();
        groups.pop_front();

        // 按照一组GOP来进行解码
        decodeGOP(group);
        // 进行下一组GOP解码
    }
    decodeComplete = true;
}

void FFmpegVideoDecoder::decodeGOP(std::shared_ptr<FFDecoder::FrameGroupInfo> gop)
{
    if (video_dec_ctx == nullptr)
    {
        return;
    }

    // 步骤：
    // 偏移到I帧位置
    // 取出编码数据
    // 持续解码数据，添加到已解码缓存队列中
    // 是否需要渲染画面（消费者线程中处理）
    // 判断本组画面是否都已解码完成

    // I帧位置
    int64_t timestampsUs = gop->timestampsUs;

    AVRational time_base = fmt_ctx->streams[video_stream_idx]->time_base;
    // 时间基: 时间单位 简单理解: 1秒划分成多少份
    // eg: 10(时间基) -> 10(秒) -> 1/6(分钟) -> 0.167(分钟)
    // 1秒钟60帧,时间基 = 16ms = 1/60 成为一个时间基
    // AVPacket.pts = 56(时间基)
    // 时间基单位(时间基) = 56 * (1/60), 换算成秒 = 0.933 秒 = 933 毫秒
    // 反过来换算?
    // 1, a * bq / cq
    // 2, bq = (num/den)
    // 3, a * (num1/den1) / (num2/den2)
    // us to pts
    int64_t pts_unit = FFUtil::us_to_pts(timestampsUs, time_base);

    // seek to
    int ret = av_seek_frame(fmt_ctx, video_stream_idx, pts_unit, AVSEEK_FLAG_BACKWARD);

    if (ret < 0)
    {
        return;
    }

    // 刷新输入文件的解码器上下文
    avcodec_flush_buffers(video_dec_ctx);

    // 开始读取数据
    AVPacket *_packet = av_packet_alloc();
    AVFrame *_frame = av_frame_alloc();


    bool eof = false;
    while (av_read_frame(fmt_ctx, _packet) >= 0 && !eof)
    {
        if (_packet->stream_index == video_stream_idx)
        {

            // 解码画面
            decodeFrame(_frame, _packet, [&](AVFrame *f) {

                // 从待解码视频队列中取出第一个，这里的时间戳顺序都是从小到大的
                int64_t pts = gop->frames.front();
                // pts to us
                int64_t pts_us = FFUtil::pts_to_us(f->pts, time_base);
                // 获取到对应视频帧的编码数据
                if (pts_us >= pts)
                {
                    // 命中解码视频，从待解码队列中移除
                    gop->frames.pop_front();
                    // 处理画面
                    f->time_base = time_base; // 手动补充时间基,后面拷贝frame用到
                    handleFrame(f);
                }

                // 判断本组是否已完成
                eof = gop->frames.empty();

            });

        }
    }

    // 清空解码器中的残余buffer，为下一次解码做准备
    avcodec_flush_buffers(video_dec_ctx);

    av_packet_free(&_packet);
    av_frame_free(&_frame);

}

void FFmpegVideoDecoder::decodeFrame(AVFrame *frame, AVPacket *pkt,
                                     std::function<void(AVFrame *)> func)
{
    // 编码数据送给解码器解码
    int ret = avcodec_send_packet(video_dec_ctx, pkt);

    // GOP: group of pictures
//    {"I","P","P","P","B","P","P","P","B","P","P","B","P","P","B","B","P"}
//    {"I","P","P","P","B","P","P","P","B","P","P","B"} -> { "B" }
    // 循环读取已解码数据
    while (ret >= 0)
    {
        ret = avcodec_receive_frame(video_dec_ctx, frame);
        if (ret == AVERROR(EAGAIN))
        {
            // 当前帧不足以解码出来，需要用户添加新帧
            // eg：遇到B帧的时候会触发这个情况
            return;
        } else if (ret == AVERROR_EOF)
        {
            // end of file
            return;
        } else if (ret < 0)
        {
            // 这里就是出现错误了
            logger.e("Error during decoding");
            return;
        }

        // 外部处理解码数据
        func(frame);
    }

}

void FFmpegVideoDecoder::handleFrame(AVFrame *frame)
{
    // 使用独占式智能指针 std::unique_ptr<> 管理内存释放
    AVFrameImage* avFrameImage = new AVFrameImage(frame, true);
    std::unique_ptr<TexImage> image(dynamic_cast<TexImage *>(avFrameImage));
    decodedFrames.push(std::move(image)); // 从尾部加入
    if (shouldAutoRender) // 需要渲染了
    {
        shouldAutoRender = false;
        renderOnce();
    }
}

void FFmpegVideoDecoder::renderOnce()
{
    // 数据拷贝一份，送去渲染
    if (canvas != nullptr && !decodedFrames.empty())
    {
        // 从已解码队列中取出
        std::unique_ptr<TexImage> image = std::move(decodedFrames.front());
        decodedFrames.pop();
        // 通知上层
        if (jniCaller != nullptr)
        {
            int64_t timestampsUs = image->getTimestampsUs();
            jniCaller->onRender(timestampsUs);
        }
        canvas->draw(std::move(image)); // 使用移动语义，转移所有权
    } else if (decodeComplete)
    {
        // 解码任务都完成了，待渲染的视频帧也没有了，此时回调结束
        if (jniCaller != nullptr)
        {
            jniCaller->onComplete();
        }
    } else
    {
        // 没有内容了，但是解码还在继续
        // 等待下一帧解码出来的数据，自动送显
        shouldAutoRender = true;
    }
}