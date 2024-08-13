//
// Created by liaohailong on 2024/6/13.
//

#include "HelloEGLContext.hpp"


HelloEGLContext::HelloEGLContext() : logger("HelloEGLContext"), width(-1), height(-1)
{
    logger.i("HelloEGLContext::HelloEGLContext()");
}

HelloEGLContext::~HelloEGLContext()
{
    // 释放EGL环境资源
    // 移除操作参考：Android平台的GLSurfaceView.java
    eglMakeCurrent(eglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    eglDestroySurface(eglDisplay, eglSurface);
    if (!eglDestroyContext(eglDisplay, eglContext))
    {
        EGLint error = eglGetError();
        logger.i("eglDestroyContext error[%d]", error);
    }
    eglTerminate(eglDisplay);

    logger.i("HelloEGLContext::~HelloEGLContext()");
}

int HelloEGLContext::init(void *sharedContext)
{
    eglDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);

    int major[1];
    int minor[1];
    if (!eglInitialize(eglDisplay, major, minor))
    {
        logger.i("eglInitialize error : %d", eglGetError());
        return -1;
    }

    logger.i("eglInitialize success major = %d, minor = %d", major[0], minor[0]);

    int attr_list[] = {
            EGL_RED_SIZE, 8,
            EGL_GREEN_SIZE, 8,
            EGL_BLUE_SIZE, 8,
            EGL_ALPHA_SIZE, 8,
            EGL_BUFFER_SIZE, 32, // 单个像素buffer大小
            EGL_DEPTH_SIZE, 16, // 深度大小，3D渲染时必须设置
            EGL_RENDERABLE_TYPE, EGL_OPENGL_ES3_BIT_KHR, // EGL配置支持OpenGL ES3.0
            EGL_SURFACE_TYPE, EGL_WINDOW_BIT | EGL_PBUFFER_BIT, // surface类型支持：窗口渲染，离屏渲染
            EGL_NONE // 配置结束符号，类似字符串中的 \0 用作系统识别来终止继续读取
    };
    int num = 1;
    EGLConfig configs[num];
    int num_config[num];

    // 配置属性，得到EGLConfig，后续创建 EGLSurface 会用到
    if (!eglChooseConfig(eglDisplay, attr_list, configs, num, num_config))
    {
        logger.i("eglChooseConfig error : %d", eglGetError());
        return -1;
    }

    eglConfig = configs[0];

    int attr_list2[] = {
            EGL_CONTEXT_CLIENT_VERSION, 3, // 使用OpenGL ES3.0接口编程
            EGL_NONE
    };

    if (sharedContext == nullptr)
    {
        sharedContext = EGL_NO_CONTEXT;
    }
    // 创建EGL上下文，这里第三个参数参数如果有值，则会使这两个渲染线程共用一个EGL上下文，能共享纹理
    eglContext = eglCreateContext(eglDisplay, eglConfig, sharedContext, attr_list2);
    if (eglContext == EGL_NO_CONTEXT)
    {
        logger.i("eglCreateContext error : %d", eglGetError());
        return -1;
    }

    logger.i("eglCreateContext created success!");
    return 0;
}

int HelloEGLContext::setSurface(void *_surface)
{
    ANativeWindow *surface = static_cast<ANativeWindow *>(_surface);
    width = ANativeWindow_getWidth(surface);
    height = ANativeWindow_getHeight(surface);

    int attr_list[] = {
            EGL_NONE
    };
    // 将上层传递下来的surface制作成 EGLSurface
    eglSurface = eglCreateWindowSurface(eglDisplay, eglConfig, surface, attr_list);
    // 确定使用当前 EGLSurface 作为前置缓冲区buffer运送的目的地
    if (!eglMakeCurrent(eglDisplay, eglSurface, eglSurface, eglContext))
    {
        logger.i("eglMakeCurrent error : %d", eglGetError());
        return -1;
    }

    logger.i("eglMakeCurrent success!");


    // 把绘制内容输出到多个output上
//    EGLSurface eglSurfaceArr[] = {eglSurface1,eglSurface2,eglSurface3,eglSurface4};
//    for (const auto &item: eglSurfaceArr)
//    {
//        eglMakeCurrent
//        draw OpenGL ES
//        eglSwapBuffers()
//    }

    // 下面这个接口是 Android 平台特有，跟 MediaCodec 搭配使用，当渲染缓冲区交换之后，调用此函数，通知 MediaCodec 进行编码
    // 检查eglPresentationTimeANDROID函数是否有效
    auto pfneglPresentationTimeANDROID
            = reinterpret_cast<PFNEGLPRESENTATIONTIMEANDROIDPROC>(eglGetProcAddress(
                    "eglPresentationTimeANDROID"));
    if (!pfneglPresentationTimeANDROID)
    {
        logger.i("eglPresentationTimeANDROID is not available!");
    } else
        logger.i("eglPresentationTimeANDROID is available!");
    return 0;
}

void HelloEGLContext::renderStart()
{
    // 每次绘制之前，需要清空缓冲区内容，不然会在上一次的基础上绘制，导致雪花屏
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
}

bool HelloEGLContext::renderEnd()
{
    return renderEnd(-1);
}

bool HelloEGLContext::renderEnd(int64_t ptsUs)
{
    // 着色器代码运行完毕，绘制好内容之后，交换前后缓冲区，让画面更新
    bool b1 = eglSwapBuffers(eglDisplay, eglSurface);
    bool b2 = true;
    if (ptsUs >= 0)
    {
        // 通知 MediaCodec 编码画面内容
        b2 = eglPresentationTimeANDROID(eglDisplay, eglSurface, ptsUs);
    }
    return b1 && b2;
}

int HelloEGLContext::getWidth()
{
    return width;
}

int HelloEGLContext::getHeight()
{
    return height;
}