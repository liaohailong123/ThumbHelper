//
// Created by liaohailong on 2024/6/16.
//

#ifndef HELLOGL_FFMPEGVIDEODECODER_HPP
#define HELLOGL_FFMPEGVIDEODECODER_HPP

#include <string>

extern "C"
{
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/dict.h>
}


#include "../util/LogUtil.hpp" //log日志打印
#include "../handler/NativeHandler.hpp" // 消息驱动模式，多线程编程
#include "../render/canvas/YUVCanvas.hpp" // YUV绘制画布，使用OpenGLES绘制YUV纹理
#include "../render/entity/AVFrameImage.hpp" // 纹理内容（包裹了ffmpeg库的AVFrame对象）
#include "../util/FFUtil.hpp" // ffmpeg工具库


#include <android/native_window.h> // Android上层android.view.Surface
#include <android/native_window_jni.h> // Surface 与 ANativeWindow 相互转换
#include <list> // std::list 双向链表
#include <queue> // std::queue 队列结构
#include <unordered_map> // std::unordered_map<>集合
#include <memory> // std::shared_ptr<> 共享智能指针 std::unique_ptr<> 独占式智能指针
#include <functional> // 函数封装
#include <atomic> // 原子操作


namespace FFDecoder
{
    struct FrameGroupInfo
    {
        /**
         * seekTo的时间戳，单位：us
         */
        int64_t timestampsUs;

        /**
         * 画面帧的时间戳，单位：us
         */
        std::list<int64_t> frames;
    };
}

/**
 * C++层回调Java层
 */
class FFmpegVideoDecoderJniCaller
{
public:
    FFmpegVideoDecoderJniCaller(jobject instance);

    ~FFmpegVideoDecoderJniCaller();

    /**
     * @param timestampsUs 当前画面帧已经渲染，单位：us
     */
    void onRender(int64_t timestampsUs);

    /**
     * 所有渲染任务已经全部结束
     */
    void onComplete();

private:
    jobject globalObj;

};

/**
 * create by liaohailong
 * 2024/6/16 16:39
 * desc: 
 */
class FFmpegVideoDecoder
{
public:
    FFmpegVideoDecoder();

    ~FFmpegVideoDecoder();

    /**
     * @param instance 设置Java层实例，JNI回调
     */
    void setJniInstance(jobject instance);

    /**
     * @param uri 资源路径
     * @return 0=success <0=error
     */
    int setDataSource(const char *uri);

    /**
     * @param surface 设置画面输出结果
     */
    void setSurface(ANativeWindow *surface);

    /**
     * @param timestampsUs 获取当前时间戳，最近的一个I帧（向前取）
     * @return I帧的时间戳，单位：us
     */
    int64_t getPreviousIFrameUs(int64_t timestampsUs);

    /**
     *
     * @param frameUs 需要解码的视频帧数组，单位：us
     * @param count 数组长度
     * @param exactly true=精准模式
     */
    void start(int64_t *frameUs, int count, bool exactly);

    /**
     * 开始解码所有视频帧
     */
    void decodeAll();

    /**
     * @param gop 解码一组GOP视频帧
     */
    void decodeGOP(std::shared_ptr<FFDecoder::FrameGroupInfo> gop);

    /**
     * 解码一帧画面
     * @param frame 解码数据
     * @param pkt 编码数据（待解码）
     * @param func 解码成功回调
     */
    void decodeFrame(AVFrame *frame, AVPacket *pkt, std::function<void(AVFrame *)> func);

    /**
     * @param frame 处理一帧画面，加入待渲染队列中
     */
    void handleFrame(AVFrame *frame);

    /**
     * 送显到 surface 上
     */
    void renderOnce();

private:
    int open_codec_context(int *stream_idx, AVCodecContext **dec_ctx, AVFormatContext *fmt_ctx,
                           enum AVMediaType type);

private:
    Logger logger; // log日志输出

    FFmpegVideoDecoderJniCaller *jniCaller; // C++回调Java
    std::string uri; // 资源路径
    ANativeWindow *surface; // 渲染缓冲区

    // ffmpeg相关
    AVFormatContext *fmt_ctx; // 拆封装上下文,取出多媒体封装文件中
    AVCodecContext *video_dec_ctx;
    int video_stream_idx;

    std::list<std::shared_ptr<FFDecoder::FrameGroupInfo>> groups; // 当前渲染的画面分组

    NHandlerThread *decodeThread; // 解码线程
    NHandler *decodeHandler; // 解码Handler
    std::queue<std::unique_ptr<TexImage>> decodedFrames;

    YUVCanvas *canvas; // YUV画布，用来渲染YUV数据到surface上
    std::atomic<bool> shouldAutoRender; // 自动渲染解码数据（第一帧 或 需要渲染但无解码数据时）
    std::atomic<bool> decodeComplete; // 所有视频数据解码完毕

};


#endif //HELLOGL_FFMPEGVIDEODECODER_HPP
