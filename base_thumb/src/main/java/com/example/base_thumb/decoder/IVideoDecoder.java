package com.example.base_thumb.decoder;

import android.view.Surface;

import androidx.annotation.NonNull;

import com.example.base_thumb.entity.FrameInfo;

import java.util.List;

/**
 * create by liaohailong
 * 2024/6/13 21:34
 * desc: 统一解码器规范
 */
public interface IVideoDecoder {
    static IVideoDecoder newInstance(boolean hard) {
        IVideoDecoder decoder;
        if (hard) {
            decoder = new SystemVideoDecoder();
        } else {
            decoder = new FFmpegVideoDecoder();
        }
        return decoder;
    }

    /**
     * @param uri 设置多媒体资源
     */
    void setDataSource(String uri);

    /**
     * @param surface 设置解码输出位置 surface OES/SurfaceView
     */
    void setSurface(Surface surface);

    /**
     * @param callback 设置回调
     */
    void setCallback(Callback callback);

    /**
     * 开始解码
     * @param frames 需要解码的视频帧
     * @param exactly true表示精准模式，速度慢，画面精准 false表示高效模式，速度快，可能画面不准
     */
    void start(@NonNull List<FrameInfo> frames, boolean exactly) throws Exception;

    /**
     * 继续获取下一帧解码数据
     */
    void dequeueNextFrame();

    /**
     * 释放资源
     */
    void release();


    interface Callback{
        void onRender(long timestampsUs);

        void onComplete();
    }

}
