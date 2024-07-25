package com.example.base_thumb.decoder;

import android.os.Handler;
import android.os.HandlerThread;
import android.util.Log;
import android.view.Surface;

import androidx.annotation.Keep;
import androidx.annotation.NonNull;

import com.example.base_thumb.entity.FrameInfo;

import java.util.List;

/**
 * create by liaohailong
 * 2024/6/13 21:34
 * desc: use ffmpeg to decode
 */
public class FFmpegVideoDecoder implements IVideoDecoder {
    private static final String TAG = "FFmpegVideoDecoder";

    private static void log(String format, Object... args) {
        String msg = String.format(format, args);
        Log.i(TAG, msg);
    }

    private final long mPtr;

    private final Handler workHandler;
    private Callback callback;

    public FFmpegVideoDecoder() {
        this.mPtr = nativeInit();

        HandlerThread ht = new HandlerThread("ffmpeg-video-decoder");
        ht.start();
        workHandler = new Handler(ht.getLooper());
    }

    @Override
    public void setDataSource(String uri) {
        if (mPtr != -1) {
            workHandler.post(() -> {
                int ret = nativeSetDataSource(mPtr, uri);
                if (ret == -1) {
                    // 出现问题了
                    log("nativeSetDataSource error: %d", ret);
                }
            });
        }
    }

    @Override
    public void setSurface(Surface surface) {
        workHandler.post(() -> {
            if (mPtr != -1) {
                nativeSetSurface(mPtr, surface);
            }
        });
    }

    @Override
    public void setCallback(Callback callback) {
        this.callback = callback;
    }

    @Override
    public void start(@NonNull List<FrameInfo> frames, boolean exactly) {
        workHandler.post(() -> {
            if (mPtr != -1) {
                // 所有需要解析画面的时间戳传递给C++层处理
                long[] frameTimestamps = new long[frames.size()];
                for (int i = 0; i < frameTimestamps.length; i++) {
                    frameTimestamps[i] = frames.get(i).getTimestampsUs();
                }
                nativeStart(mPtr, frameTimestamps, exactly);
            }
        });

    }

    @Override
    public void dequeueNextFrame() {
        if (mPtr != -1) {
            nativeDequeueNextFrame(mPtr);
        }
    }

    @Override
    public void release() {
        if (mPtr != -1) {
            nativeDestroy(mPtr);
        }
        workHandler.removeCallbacksAndMessages(null);
        workHandler.getLooper().quit();
    }

    private native long nativeInit();

    private native int nativeSetDataSource(long ptr, String uri);

    private native void nativeSetSurface(long ptr, Surface surface);

    private native void nativeStart(long ptr, long[] frameUs, boolean exactly);

    private native void nativeDequeueNextFrame(long ptr);

    private native void nativeDestroy(long ptr);

    /**
     * called by native code
     *
     * @param timestampsUs 当前渲染的视频时间戳，单位：us
     */
    @Keep
    private void onRender(long timestampsUs) {
        if (callback != null) {
            callback.onRender(timestampsUs);
        }
    }

    /**
     * called by native code
     * 解码任务已全部完成
     */
    @Keep
    private void onComplete() {
        if (callback != null) {
            callback.onComplete();
        }
    }
}
