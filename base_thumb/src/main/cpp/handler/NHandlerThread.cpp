//
// Created by liaohailong on 2024/6/2.
//


#include "NativeHandler.hpp"


NHandlerThread::NHandlerThread(std::string _name, HandlerMessage _callback) : name(_name),
                                                                              callback(_callback),
                                                                              onHandler(nullptr),
                                                                              thread(nullptr),
                                                                              args(nullptr),
                                                                              handler(nullptr)
{
    NHLog::instance()->i("NHandlerThread[%s] 构造函数执行", name.c_str());
}

NHandlerThread::~NHandlerThread()
{
    NHLog::instance()->i("~NHandlerThread[%s] 析构函数执行", name.c_str());
    quite();
}

void NHandlerThread::start(OnHandlerCallback r, void *_args)
{
    if (thread == nullptr)
    {
        args = _args;
        onHandler = r;
        thread = new std::thread(&NHandlerThread::run, std::ref(*this));
    }
}

void NHandlerThread::run()
{
    NLooper::prepare();

    handler = new NHandler(callback);
    if (onHandler != nullptr)
    {
        onHandler(handler, args);
    }

    NLooper::loop();
}

NHandler *NHandlerThread::getHandler()
{
    return handler;
}

void NHandlerThread::quite()
{
    NLooper *looper = nullptr;
    if (thread != nullptr)
    {
        if (handler != nullptr)
        {
            looper = handler->getLooper();
            if (looper != nullptr)
            {
                looper->quite();
            }
            delete handler;
            handler = nullptr;
        }
        // 一般都是其他线程调用 quite 函数
        if (std::this_thread::get_id() != thread->get_id())
        {
            // 等待 NLooper::loop 结束
            if (thread->joinable())
            {
                thread->join();
            }
        } else
        {
            // 在本线程内执行 quite 的情况兼容一下
            thread->detach();
        }

        if (looper != nullptr)
        {
            delete looper;
            looper = nullptr;
        }
        thread = nullptr;
    }
}