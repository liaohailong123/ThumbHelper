//
// Created by liaohailong on 2024/6/21.
//

#ifndef HELLOGL_ICANVAS_HPP
#define HELLOGL_ICANVAS_HPP

#include <stdio.h>
#include <android/native_window.h>
#include <android/native_window_jni.h>
#include <memory> // std::unique_ptr<>
#include <queue> // std::queue<> 队列

#include "../entity/TexImage.hpp"
#include "../../util/LogUtil.hpp"
#include "../../handler/NativeHandler.hpp"
#include "../HelloEGLContext.hpp"
#include "../program/YUVGLProgram.hpp"

/**
 * create by liaohailong
 * 2024/6/21 22:18
 * desc: 画布基类
 */
class ICanvas
{
public:
    ICanvas(ANativeWindow *surface, int width, int height, const char *tag = "ICanvas");

    virtual ~ICanvas();

    /**
     * 渲染线程 和 EGL环境创建好后回调
     */
    virtual void onEGLCreated() = 0;

    /**
     * @param image 渲染内容
     */
    virtual void draw(std::unique_ptr<TexImage> image) = 0;

protected:
    Logger logger;
    ANativeWindow *surface; // 渲染surface
    int width, height; // 视口大小
    NHandlerThread *renderThread; // 渲染线程
    NHandler *renderHandler; // 渲染Handler

    HelloEGLContext *glContext; // EGL环境
    IGLProgram *glProgram; // YUV着色器程序

    std::queue<std::unique_ptr<TexImage>> imageQueue;

};


#endif //HELLOGL_ICANVAS_HPP
