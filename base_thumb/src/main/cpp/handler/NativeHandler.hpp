//
// Created by liaohailong on 2024/6/1.
//

#ifndef HELLOC_NATIVEHANDLER_HPP
#define HELLOC_NATIVEHANDLER_HPP


#include <unistd.h> // pipe() pipe2()
#include <string>
#include <sys/select.h> // select()
#include <thread> // 子线程
#include <mutex> // 互斥锁
#include <atomic> // 原子操作
#include <unordered_map> // map
#include <vector> // 可扩容数组
#include <memory> // 智能指针
#include <cstdarg> // 取出可扩展参数 ...

#define NUM(arr) (sizeof(arr)/sizeof(arr[0]))

struct NMessage;

class NHandler;

/**
 * 打印 log 信息
 * NativeHandlerLog
 */
class NHLog
{
public:
    explicit NHLog() = default;

    virtual  ~NHLog() = default;

    // log日志的全局默认tag
    const char *defaultTag = "NHLog";

    // 外部注册的 log 打印函数
    int (*onLogPrint)(const char *tag, const char *format, ...);


    int i(const char *format, ...)
    {
        va_list args;
        va_start(args, format);
        int status = i(defaultTag, format, args);
        va_end(args);
        return status;
    }

    int i(const char *tag, const char *format, ...)
    {
        if (onLogPrint != nullptr)
        {
            va_list args;
            va_start(args, format);
            int status = onLogPrint(tag, format, args);
            va_end(args);
            return status;
        }

        return -1;
    }

private:
    static std::once_flag flag; // C++11标准库，多线程安全，只调用一次
    static NHLog *sInstance;

    static void CreateInstance()
    {
        if (sInstance == nullptr)
        {
            sInstance = new NHLog();
        }
    }

public:
    static NHLog *instance()
    {
        std::call_once(flag, CreateInstance);
        return sInstance;
    }

};


/**
 * 可执行任务Runnable
 * @param args 携带参数
 */
typedef void(*Runnable)(void *args);

/**
 * NHandler中处理NMessage，类似Android中Handler重写dispatchMessage函数
 */
typedef void(*HandlerMessage)(NMessage *msg);

/**
 * NHandlerThread开启后，内部创建好NHandler回调此函数
 */
typedef void(*OnHandlerCallback)(NHandler *handler, void *args);

namespace NSystemClock
{
    /**
     * @return 获取当前时间戳，单位：ms
     */
    long uptimeMillis();
}

/**
 * 消息结构体
 */
struct NMessage
{
    int what; // 消息标记
    Runnable func; // 执行函数
    void *args; // 传递给func的参数
    long when; // 执行时间戳，单位：ms
    NHandler *handler;

    NMessage *next;
};

/**
 * 低优先级任务处理，MessageQueue空闲时执行
 */
class NIdleHandler
{
public:
    NIdleHandler()
    {
        NHLog::instance()->i("NIdleHandler() address: %p", this);
    }

    virtual ~NIdleHandler()
    {
        NHLog::instance()->i("~NIdleHandler() address: %p", this);
    }

    /**
     * 当消息队列中没有可执行的消息时，处于一个空闲状态，会回调此函数
     * @return true表示消息队列继续保留此IdleHandler，false表示执行完毕后就会被移除掉
     */
    virtual bool queueIdle()
    {
        return false;
    };
};

/**
 * 消息队列
 */
class NMessageQueue
{
public:
    NMessageQueue();

    ~NMessageQueue();

    /**
     * 把一个消息入队
     * @param msg 消息
     * @param when 消息执行的时间戳，单位：ms
     * @return true表示入队成功
     */
    bool enqueueMessage(NMessage *msg, long when);

    /**
     * 把一个带有Runnable的消息移除队列
     * @param h handler对象指针
     * @param r 可执行的Runnable任务
     */
    void removeMessages(NHandler *h, Runnable r);

    /**
     * 把所有what（标记）的消息移除队列
     * @param h handler对象指针
     * @param what 消息标记
     */
    void removeMessages(NHandler *h, int what);

    /**
     * 把空闲处理者加入消息队列，当 NIdleHandler.queueIdle 返回false时会被移除掉
     * @param handler 空闲时的处理者
     */
    void addIdleHandler(std::shared_ptr<NIdleHandler> handler);

    /**
     * @param handler 移除掉空闲时处理者
     */
    void removeIdleHandler(std::shared_ptr<NIdleHandler> handler);

    /**
     * @return 返回下一个消息，返回nullptr表示消息队列已关闭
     */
    NMessage *next();

    /**
     * 退出/关闭消息队列
     */
    void quit();

private:
    /**
     * 初始化
     */
    void init();

    /**
     * 释放资源
     */
    void destroy();

    /**
     * 等待事件，可根据系统平台分别实现
     * @param timeoutMillis 等待最长时间，-1表示一直等待，单位：ms
     */
    void pollOnce(int timeoutMillis);

    /**
     * 唤醒等待
     */
    void wake();

private:
    /**
     * 监听句柄
     */
    int fd[2];
    std::vector<std::shared_ptr<NIdleHandler>> idleHandlers;
    NMessage *message;
    std::atomic<bool> quitting;
    std::mutex mutex;
    std::atomic<bool> blocked;

};

/**
 * 消息循环器，从MessageQueue中获取Message
 */
class NLooper
{
public:
    NLooper();

    ~NLooper();

    /**
     * 初始化Looper，根据当前线程，创建looper并存储
     */
    static void prepare();

    /**
     * 消息循环，从消息队列中取出消息并执行
     */
    static void loop();

    /**
     * 尝试获取一个消息来执行
     * @param me 当前looper
     * @return true表示成功取出消息
     */
    static bool loopOnce(NLooper *me);

    /**
     * 退出循环
     */
    void quite();

    /**
     * @return 获取消息队列
     */
    NMessageQueue *getQueue();

    /**
     * @return 获取当前线程中的looper，可能为空，需要先调用 [prepare] 函数
     */
    static NLooper *myLooper();

public:
    static std::unordered_map<std::__thread_id, NLooper *> sThreadLocal;
    static std::mutex sMutex;


private:
    NMessageQueue *messageQueue;
    std::atomic<bool> looping;
};

/**
 * create by liaohailong
 * 2024/6/1 18:54
 * desc: 使用 pipe + select ，仿写一个简单的 Handler 机制
 */
class NHandler
{
public:
    explicit NHandler(HandlerMessage = nullptr);

    ~NHandler();

    /**
     * 创建Message
     * @param r 可执行任务
     * @param args 传给Runnable的参数
     * @return 创建好的Message
     */
    NMessage *obtainMessage(Runnable &r, void *args);

    /**
     * 创建Message
     * @param what 消息标记
     * @param args 携带参数
     * @return 创建好的Message
     */
    NMessage *obtainMessage(int what, void *args = nullptr);

    /**
     * @param r 添加执行任务
     * @param args 携带参数
     * @return true表示添加成功
     */
    bool post(Runnable r, void *args = nullptr);

    /**
     * @param r 添加执行任务，插队执行（接下来马上就执行）
     * @param args 携带参数
     * @return true表示添加成功
     */
    bool postAtFrontOfQueue(Runnable r, void *args = nullptr);

    /**
     * @param r 添加执行任务
     * @param uptimeMillis 指定在哪一时间执行，单位：ms
     * @param args 携带参数
     * @return true表示添加成功
     */
    bool postAtTime(Runnable r, long uptimeMillis, void *args = nullptr);

    /**
     * @param r 添加执行任务
     * @param delayMillis 多少毫秒之后执行
     * @param args 携带参数
     * @return true表示添加成功
     */
    bool postDelayed(Runnable r, long delayMillis, void *args = nullptr);

    /**
     * @param r 移除之前添加的执行任务
     */
    void removeCallbacks(Runnable r);

    /**
     * @param msg 发送一个消息，在[onDispatchMessage]中执行
     * @return true表示添加成功
     */
    bool sendMessage(NMessage *msg);

    /**
     * @param msg 发送一个消息（插队），在[onDispatchMessage]中执行
     * @return true表示添加成功
     */
    bool sendMessageAtFrontOfQueue(NMessage *msg);

    /**
     * @param msg
     * @param uptimeMillis 指定在哪一时间执行，单位：ms
     * @return true表示添加成功
     */
    bool sendMessageAtTime(NMessage *msg, long uptimeMillis);

    /**
     * @param msg 发送一个消息
     * @param delayMillis 多少毫秒之后执行
     * @return true表示添加成功
     */
    bool sendMessageDelayed(NMessage *msg, long delayMillis);

    /**
     * @param what 消息的标记
     * @return true表示添加成功
     */
    bool sendEmptyMessage(int what);

    /**
     * @param what 消息的标记
     * @param uptimeMillis
     * @return true表示添加成功
     */
    bool sendEmptyMessageAtTime(int what, long uptimeMillis);

    /**
     * @param what 消息的标记
     * @param delayMillis
     * @return true表示添加成功
     */
    bool sendEmptyMessageDelayed(int what, long delayMillis);

    /**
     * @param what 移除某种标记的消息
     */
    void removeMessages(int what);

    /**
     * 消息入队列
     * @param queue 消息队列对象指针
     * @param msg 消息
     * @param uptimeMillis 入队时间（执行该消息的时间），单位：ms
     * @return true表示入队列成功
     */
    bool enqueueMessage(NMessageQueue *queue, NMessage *msg, long uptimeMillis);

    /**
     * @param msg 消息分发
     */
    void dispatchMessage(NMessage *msg);

    /**
     * @return 获取looper对象指针
     */
    NLooper *getLooper();

private:
    /**
     * 消息分发回调外部
     */
    HandlerMessage onDispatchMessage;
    /**
     * 消息循环对象
     */
    NLooper *looper;
    /**
     * 消息队列
     */
    NMessageQueue *messageQueue;
};

/**
 * 一个子线程，用来创建NHandler
 */
class NHandlerThread
{
public:
    /**
     * @param name 子线程的名称
     * @param callback 消息分发回调，这个callback会传递给NHandler
     */
    explicit NHandlerThread(std::string name, HandlerMessage callback = nullptr);

    ~NHandlerThread();

    /**
     * 启动Handler子线程
     * @param r NHandler创建成功后回调
     * @param args 回调携带参数，会传递给 OnHandlerCallback r
     */
    void start(OnHandlerCallback r = nullptr, void *args = nullptr);

    /**
     * @return 获取当前子线程的NHandler，需判空
     */
    NHandler *getHandler();

    /**
     * 子线程退出，同时内部会退出NHandler
     */
    void quite();

private:
    std::string name;
    HandlerMessage callback;
    void *args;
    OnHandlerCallback onHandler;
    std::thread *thread;
    NHandler *handler;

    /**
     * 子线程运行函数
     */
    void run();

};

#endif //HELLOC_NATIVEHANDLER_HPP
