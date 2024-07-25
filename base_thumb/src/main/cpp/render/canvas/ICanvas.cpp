//
// Created by liaohailong on 2024/6/21.
//

#include "ICanvas.hpp"

ICanvas::ICanvas(ANativeWindow *_surface, int _width, int _height, const char *tag) : logger(tag),
                                                                                      surface(_surface),
                                                                                      width(_width),
                                                                                      height(_height),
                                                                                      imageQueue()
{

    // 创建子线程，EGL环境
    std::string threadName = tag;
    threadName += "-handler-thread";

    renderThread = new NHandlerThread(threadName);
    renderThread->start([](NHandler *handler, void *args) {
        ICanvas *canvas = reinterpret_cast<ICanvas *>(args);
        canvas->renderHandler = handler;
        canvas->glContext = new HelloEGLContext();
        canvas->glContext->init(EGL_NO_CONTEXT);
        canvas->glContext->setSurface(canvas->surface);

        // EGL环境创建好
        canvas->onEGLCreated();
    }, this);
}


ICanvas::~ICanvas()
{
    // 释放资源：Handler + EGL上下文环境
    if (renderHandler != nullptr)
    {
        renderHandler->postAtFrontOfQueue([](void *args) {
            ICanvas *canvas = reinterpret_cast<ICanvas *>(args);
            if (canvas->glContext != nullptr)
            {
                delete canvas->glContext;
                canvas->glContext = nullptr;
            }
            if (canvas->glProgram != nullptr)
            {
                delete canvas->glProgram;
                canvas->glProgram = nullptr;
            }
        }, this);
        if (renderThread != nullptr)
        {
            delete renderThread;
            renderThread = nullptr;
        }
    }
}



