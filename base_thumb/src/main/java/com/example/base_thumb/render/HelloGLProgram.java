package com.example.base_thumb.render;

import androidx.annotation.NonNull;

/**
 * create by liaohailong
 * 2024/6/15 21:24
 * desc: JNI桥代码，着色器程序
 */
public class HelloGLProgram {

    private long mPtr;
    private final Type type;

    public HelloGLProgram(@NonNull Type type) {
        this.mPtr = -1;
        this.type = type;
    }

    public void init() {
        mPtr = nativeInit(type.ordinal());
    }

    public void setTexture(int texture) {
        if (mPtr != -1) {
            nativeSetTexture(mPtr, texture);
        }
    }

    public void setRotation(float rotation) {
        if (mPtr != -1) {
            nativeSetRotation(mPtr, rotation);
        }
    }

    public void setMirror(boolean hMirror, boolean vMirror) {
        if (mPtr != -1) {
            nativeSetMirror(mPtr, hMirror, vMirror);
        }
    }

    public void begin() {
        if (mPtr != -1) {
            nativeBegin(mPtr);
        }
    }

    public void draw(int width, int height, float[] projectMat) {
        if (mPtr != -1) {
            nativeDraw(mPtr, width, height, projectMat);
        }
    }

    public void end() {
        if (mPtr != -1) {
            nativeEnd(mPtr);
        }
    }

    public void destroy() {
        if (mPtr != -1) {
            nativeDestroy(mPtr);
        }
    }

    private native long nativeInit(int index);

    private native void nativeSetTexture(long ptr, int textureId);

    private native void nativeSetRotation(long ptr, float rotation);

    private native void nativeSetMirror(long ptr, boolean hMirror, boolean vMirror);

    private native boolean nativeBegin(long ptr);

    private native void nativeDraw(long ptr, int width, int height, float[] projectMat);

    private native void nativeEnd(long ptr);

    private native void nativeDestroy(long ptr);


    public enum Type {
        /**
         * OES纹理 RGBA
         */
        SAMPLER_OES,
        /**
         * sampler2D纹理 RGBA
         */
        SAMPLER_2D_RGBA,
        /**
         * sampler2D纹理 YUV420/YUV422
         */
        SAMPLER_2D_YUV;

    }
}
