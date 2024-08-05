//
// Created by liaohailong on 2024/6/13.
//

#ifndef HELLOOPENGLES_HELLOEGLCONTEXT_HPP
#define HELLOOPENGLES_HELLOEGLCONTEXT_HPP


#include "IGLContext.hpp"

#include <functional>

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <EGL/eglplatform.h>

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <GLES2/gl2platform.h>


#include <android/native_window.h>
#include <android/native_window_jni.h>

// Android输出log日志
#include "../util/LogUtil.hpp"

/**
 * create by liaohailong
 * 2024/6/13 22:25
 * desc:  EGL上下文
 */
class HelloEGLContext : public IGLContext
{
public:
    HelloEGLContext();

    ~HelloEGLContext() override;

    int init(void *sharedContext) override;

    int setSurface(void *surface) override;

    void renderStart() override;

    bool renderEnd();

    bool renderEnd(int64_t ptsUs) override;

    int getWidth() override;

    int getHeight() override;


private:
    Logger logger;
    EGLDisplay eglDisplay;
    EGLConfig eglConfig;
    EGLContext eglContext;
    EGLSurface eglSurface;

    int width;
    int height;

};


#endif //HELLOOPENGLES_HELLOEGLCONTEXT_HPP
