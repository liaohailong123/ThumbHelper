package com.example.base_thumb.reader;

import android.os.Handler;
import android.os.HandlerThread;
import android.os.Looper;
import android.text.TextUtils;
import android.view.Surface;

import androidx.annotation.NonNull;

import com.example.base_thumb.ThumbAttr;
import com.example.base_thumb.decoder.IVideoDecoder;
import com.example.base_thumb.entity.FrameInfo;

import java.util.LinkedList;

/**
 * Author: liaohailong
 * Date: 2021/8/14
 * Time: 8:53
 * Description: 视频解码器
 **/
public class ThumbReader {
    private final Handler workHandler;
    private final Handler mainHandler;

    /**
     * 抽帧任务
     */
    private final LinkedList<FrameInfo> frames = new LinkedList<>();

    /**
     * 状态回调
     */
    private Callback callback;
    /**
     * GL渲染目标
     */
    private Surface surface;

    /**
     * 资源路径
     */
    private String path;

    /**
     * true表示使用平台侧编解码器 false表示使用ffmpeg
     */
    private boolean mediacodec;

    /**
     * 视频解码器
     */
    private IVideoDecoder decoder;

    public ThumbReader() {
        HandlerThread thread = new HandlerThread("thumb-decoder-" + System.currentTimeMillis());
        thread.start();
        workHandler = new Handler(thread.getLooper());
        mainHandler = new Handler(Looper.getMainLooper());
    }

    public void config(@NonNull String path,
                       boolean mediacodec,
                       @NonNull Surface surface,
                       @NonNull Callback callback) {
        this.callback = callback;
        this.surface = surface;
        this.path = path;
        this.mediacodec = mediacodec;

        try {
            decoder = IVideoDecoder.newInstance(mediacodec);
            decoder.setDataSource(path);
            decoder.setSurface(surface);
            decoder.setCallback(new IVideoDecoder.Callback() {
                @Override
                public void onRender(long timestampsUs) {
                    callback.onRender(timestampsUs);
                }

                @Override
                public void onComplete() {
                    // 解码器处理完毕等待
                    callback.onComplete();
                }
            });
        } catch (Exception e) {
            post2Main(() -> callback.onFailure(e));
        }
    }

    public void start(ThumbAttr attr) {
        post2Work(() -> {
            long startTimeMs = attr.getStartTimeMs();
            long endTimeMs = attr.getEndTimeMs();
            int count = attr.getCount();
            boolean exactly = attr.isExactly();
            boolean mediacodec = attr.isMediacodec();

            // 计算出取帧时间
            long clipDurationUs = (endTimeMs - startTimeMs) * 1000L;
            long intervalUs = clipDurationUs / count;
            frames.clear();
            for (int i = 0; i < count; i++) {
                long timestampsUs = startTimeMs * 1000L + intervalUs * i;
                frames.add(new FrameInfo(timestampsUs));
            }

            try {
                decoder.start(frames, exactly);
            } catch (Exception e) {
                if (callback != null) callback.onFailure(e);
            }
        });
    }

    public void decodeNextFrame() {
        post2Work(() -> {
            if (decoder != null) decoder.dequeueNextFrame();
        });
    }

    public void release() {
        if (workHandler != null) workHandler.getLooper().quitSafely();
        if (decoder != null) decoder.release();
    }

    private void post2Work(@NonNull Runnable runnable) {
        workHandler.post(runnable);
    }

    private void post2Main(@NonNull Runnable runnable) {
        mainHandler.post(runnable);
    }

    public interface Callback {

        void onFailure(@NonNull Exception e);

        void onRender(long timestamps);

        void onComplete();
    }
}
