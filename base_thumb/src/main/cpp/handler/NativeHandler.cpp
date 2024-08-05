//
// Created by liaohailong on 2024/6/1.
//

#include "NativeHandler.hpp"

// log日志初始化
std::once_flag NHLog::flag;
NHLog *NHLog::sInstance = nullptr;

long NSystemClock::uptimeMillis()
{
    using namespace std;
    auto now = chrono::system_clock::now();
    return chrono::duration_cast<chrono::milliseconds>(now.time_since_epoch()).count();
}

NHandler::NHandler(HandlerMessage callback) : onDispatchMessage(callback),
                                              looper(NLooper::myLooper())
{
    NHLog::instance()->i("NHandler 构造函数执行");
    if (looper != nullptr)
    {
        messageQueue = looper->getQueue();
    }
}

NHandler::~NHandler()
{
    NHLog::instance()->i("~NHandler 析构函数执行");
}


NMessage *NHandler::obtainMessage(Runnable &r, void *args)
{
    NMessage *m = new NMessage;
    m->what = -1;
    m->func = r;
    m->args = args;
    m->handler = this;
    return m;
}

NMessage *NHandler::obtainMessage(int what, void *args)
{
    NMessage *m = new NMessage;
    m->what = what;
    m->args = args;
    m->handler = this;
    return m;
}

bool NHandler::sendMessage(NMessage *msg)
{
    return sendMessageDelayed(msg, 0);
}

bool NHandler::sendMessageAtFrontOfQueue(NMessage *msg)
{
    return sendMessageAtTime(msg, 0);
}

bool NHandler::sendMessageDelayed(NMessage *msg, long delayMillis)
{
    if (delayMillis < 0)
    {
        delayMillis = 0;
    }
    return sendMessageAtTime(msg, NSystemClock::uptimeMillis() + delayMillis);
}

bool NHandler::sendMessageAtTime(NMessage *msg, long uptimeMillis)
{
    if (messageQueue != nullptr)
    {
        return enqueueMessage(messageQueue, msg, uptimeMillis);
    }

    return false;
}

bool NHandler::sendEmptyMessage(int what)
{
    return sendMessageDelayed(obtainMessage(what), 0);
}

bool NHandler::sendEmptyMessageDelayed(int what, long delayMillis)
{
    return sendMessageDelayed(obtainMessage(what), delayMillis);
}

bool NHandler::sendEmptyMessageAtTime(int what, long uptimeMillis)
{
    return sendMessageAtTime(obtainMessage(what), uptimeMillis);
}

bool NHandler::post(Runnable r, void *args)
{
    return sendMessageDelayed(obtainMessage(r, args), 0);
}

bool NHandler::postAtTime(Runnable r, long uptimeMillis, void *args)
{
    return sendMessageAtTime(obtainMessage(r, args), uptimeMillis);
}

bool NHandler::postDelayed(Runnable r, long delayMillis, void *args)
{
    return sendMessageDelayed(obtainMessage(r, args), delayMillis);
}

bool NHandler::postAtFrontOfQueue(Runnable r, void *args)
{
    return sendMessageAtFrontOfQueue(obtainMessage(r, args));
}

bool NHandler::enqueueMessage(NMessageQueue *queue, NMessage *msg, long uptimeMillis)
{
    if (queue != nullptr)
    {
        return queue->enqueueMessage(msg, uptimeMillis);
    }
    return false;
}

void NHandler::removeCallbacks(Runnable r)
{
    if (messageQueue != nullptr)
    {
        messageQueue->removeMessages(this, r);
    }
}

void NHandler::removeMessages(int what)
{
    if (messageQueue != nullptr)
    {
        messageQueue->removeMessages(this, what);
    }
}

void NHandler::dispatchMessage(NMessage *msg)
{
    if (msg->func != nullptr)
    {
        msg->func(msg->args);
    } else if (onDispatchMessage != nullptr)
    {
        onDispatchMessage(msg);
    }
}

NLooper *NHandler::getLooper()
{
    return looper;
}



