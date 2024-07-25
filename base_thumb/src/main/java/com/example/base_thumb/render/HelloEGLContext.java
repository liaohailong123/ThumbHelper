package com.example.base_thumb.render;

import android.view.Surface;

/**
 * Author: liaohailong
 * Date: 2024/6/14
 * Time: 11:30
 * Description: EGL上下文相关
 **/
public class HelloEGLContext {

    /**
     * used by native code
     */
    private final long mPtr;

    public HelloEGLContext() {
        this.mPtr = nativeInit();
    }

    public boolean setSurface(Surface surface) {
        if (mPtr != -1) {
            return nativeSetSurface(mPtr, surface) == 0;
        }
        return false;
    }

    public void renderStart() {
        if (mPtr != -1) {
            nativeRenderStart(mPtr);
        }
    }

    public void renderEnd() {
        if (mPtr != -1) {
            nativeRenderEnd(mPtr, -1);
        }
    }

    public void renderEnd(long pts) {
        if (mPtr != -1) {
            nativeRenderEnd(mPtr, pts);
        }
    }

    public int getWidth() {
        if (mPtr != -1) {
            return nativeGetWidth(mPtr);
        }
        return -1;
    }

    public int getHeight() {
        if (mPtr != -1) {
            return nativeGetHeight(mPtr);
        }
        return -1;
    }

    public void destroy() {
        if (mPtr != -1) {
            nativeDestroy(mPtr);
        }
    }


    private native long nativeInit();

    private native int nativeSetSurface(long ptr, Surface surface);

    private native void nativeRenderStart(long ptr);

    private native boolean nativeRenderEnd(long ptr, long pts);

    private native int nativeGetWidth(long ptr);

    private native int nativeGetHeight(long ptr);

    private native void nativeDestroy(long ptr);

}
