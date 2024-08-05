//
// Created by liaohailong on 2024/6/1.
//

#include "NativeHandler.hpp"

std::unordered_map<std::__thread_id, NLooper *> NLooper::sThreadLocal = {};
std::mutex NLooper::sMutex = {};

NLooper::NLooper() : messageQueue(new NMessageQueue()), looping(false)
{
    NHLog::instance()->i("NLooper 构造函数执行");
}

NLooper::~NLooper()
{
    NHLog::instance()->i("~NLooper 析构函数执行");
    if (messageQueue != nullptr)
    {
        delete messageQueue;
    }
}

NLooper *NLooper::myLooper()
{
    // 挂锁，防止多线程执行prepare函数时，数据安全问题
    std::unique_lock<std::mutex> locker(sMutex);
    std::__thread_id tid = std::this_thread::get_id();
    auto find = sThreadLocal.find(tid);
    if (find == sThreadLocal.end())
    {
        return nullptr;
    }
    return find->second;
}

void NLooper::prepare()
{
    // 挂锁，防止多线程执行，数据安全问题
    std::unique_lock<std::mutex> locker(sMutex);
    std::__thread_id tid = std::this_thread::get_id();
    auto find = sThreadLocal.find(tid);
    if (find != sThreadLocal.end())
    {
        // 请勿在同一线程中重复 prepare
        return;
    }
    sThreadLocal.insert(std::make_pair(tid, new NLooper()));
}

void NLooper::loop()
{
    NLooper *looper = myLooper();
    if (looper == nullptr)
    {
        // no looper?
        return;
    }

    if (looper->looping)
    {
        // already looping
        return;
    }

    looper->looping = true;

    for (;;)
    {
        if (!loopOnce(looper))
        {
            return;
        }
    }

}

bool NLooper::loopOnce(NLooper *me)
{
    if (me->messageQueue == nullptr)
    {
        return false;
    }

    NMessage *msg = me->messageQueue->next();
    if (msg == nullptr)
    {
        return false;
    }
    try
    {
        msg->handler->dispatchMessage(msg);
    } catch (const std::exception &e)
    {
        // ignore
    }
    catch (...)
    {

    }

    delete msg;

    return true;
}

NMessageQueue *NLooper::getQueue()
{
    return messageQueue;
}

void NLooper::quite()
{
    if (messageQueue != nullptr)
    {
        messageQueue->quit();
    }

    // 挂锁，防止多线程执行，数据安全问题
    std::unique_lock<std::mutex> locker(sMutex);

    // 移除looper可能是在其他线程中
    for (auto iter = sThreadLocal.begin(); iter != sThreadLocal.end(); ++iter)
    {
        if (this == iter->second)
        {
            sThreadLocal.erase(iter);
            break;
        }
    }

}