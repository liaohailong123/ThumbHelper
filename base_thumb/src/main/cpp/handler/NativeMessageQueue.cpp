//
// Created by liaohailong on 2024/6/2.
//

#include "NativeHandler.hpp"


NMessageQueue::NMessageQueue()
        : idleHandlers(), message(nullptr), quitting(false),
          blocked(false)
{
    memset(fd, 0, NUM(fd));
    init();
}

NMessageQueue::~NMessageQueue()
{
    destroy();
}


void NMessageQueue::init()
{
    // 创建成功：fd中的值会>0
    // fd[0] = 读事件句柄
    // fd[1] = 写事件句柄
    if (pipe(fd) == -1)
    {
        // fd没创建出来?
        return;
    }

}


void NMessageQueue::destroy()
{
    for (int i = 0; i < NUM(fd); ++i)
    {
        if (fd[i] > 0)
        {
            close(fd[i]);
            fd[i] = 0;
        }
    }
}

void NMessageQueue::pollOnce(int timeoutMillis)
{

    fd_set read_fds;
    int read_count;
    int maxfd = fd[0] + 1;

    struct timeval timeout = {};
    // timeoutMillis 大于1000ms时需要注意：
    // timeout.tv_usec 设置的时候需要控制在 1s(1000000us)之内
    // timeout.tv_sec 设置秒
    // 否者 select 会返回-1 报参数错误
    if (timeoutMillis >= 1000)
    {
        long seconds = timeoutMillis / 1000;
        long millions = timeoutMillis % 1000;
        timeout.tv_sec = seconds;
        timeout.tv_usec = millions * 1000;
    } else
    {
        timeout.tv_usec = timeoutMillis * 1000;
    }


    FD_ZERO(&read_fds);
    FD_SET(fd[0], &read_fds);

//    LOGI("pollOnce select timeoutMillis[%d] ", timeoutMillis)
    read_count = select(maxfd, &read_fds, nullptr, nullptr,
                        timeoutMillis == -1 ? nullptr : &timeout);
//    LOGI("pollOnce select timeoutMillis[%d] count[%d] thread[%ld]", timeoutMillis, read_count, std::this_thread::get_id())
    if (read_count == -1)
    {
        const char *desc = strerror(errno);
        // 这里出现了问题
        NHLog::instance()->i("pollOnce select = -1: %s", desc);
        return;
    }

    // 读取数据
    if (read_count > 0 && FD_ISSET(fd[0], &read_fds))
    {
        char buf[1024] = {};
        ssize_t bytes_count = read(fd[0], buf, sizeof(buf));
        if (bytes_count == -1)
        {
            // 出现了问题 const char *desc = strerror(errno);
            return;
        }
    }


}

void NMessageQueue::wake()
{
    char buf[1024] = "1";
    ssize_t count = write(fd[1], buf, sizeof(buf));
    if (count == -1)
    {
        // 出现了问题 const char *desc = strerror(errno);
        return;
    }

}


bool NMessageQueue::enqueueMessage(NMessage *msg, long when)
{
    if (msg == nullptr || msg->handler == nullptr)
    {
        return false;
    }

    std::unique_lock<std::mutex> locker(mutex); // 注意上锁

    if (quitting)
    {
        delete msg;
        return false;
    }

    msg->when = when;
    NMessage *p = message;
    bool needWake = false;
    if (p == nullptr || when == 0 || when < p->when)
    {
        // New head, wake up the event queue if blocked.
        msg->next = p;
        message = msg;
        needWake = blocked;
    } else
    {
        NMessage *prev;
        for (;;)
        {
            prev = p;
            p = p->next;
            if (p == nullptr || when < p->when)
            {
                // 找到插入点
                break;
            }
        }
        // 链表插入
        msg->next = p;
        prev->next = msg;
    }

    if (needWake)
    {
        wake();
    }

    return true;
}

void NMessageQueue::removeMessages(NHandler *h, Runnable r)
{
    if (h == nullptr || r == nullptr)
    {
        return;
    }

    std::unique_lock<std::mutex> locker(mutex); // 注意上锁

    NMessage *p = message;

    // eg：
    // 链表：1 1 1 1 1 1 1 1 0 1 1 1 0 0 0 0 1 1 0 0
    // 移除元素 1
    // 先从链头开始移除：
    // 1 1 1 1 1 1 1 1 0 1 1 1 0 0 0 0 1 1 0 0
    // 0 1 1 1 0 0 0 0 1 1 0 0
    // 再从链表中移除
    // 0 0 0

    // Remove all messages at front.
    // 链表头节点移动位置
    while (p != nullptr && p->handler == h && p->func == r)
    {
        NMessage *n = p->next;
        message = n;
        delete p; // 注意：移除的节点，内存回收
        p = n;
    }

    // Remove all messages after front.
    // 链表中移除节点
    while (p != nullptr)
    {
        NMessage *n = p->next;
        if (n != nullptr)
        {
            if (n->handler == h && n->func == r)
            {
                // 断链：节点移除
                NMessage *nn = n->next;
                delete n; // 注意：移除的节点，内存回收
                p->next = nn;
                continue;
            }
        }
        p = n;
    }
}

void NMessageQueue::removeMessages(NHandler *h, int what)
{
    if (h == nullptr)
    {
        return;
    }

    std::unique_lock<std::mutex> locker(mutex); // 注意上锁

    NMessage *p = message;

    // Remove all messages at front.
    // 链表头节点移动位置
    while (p != nullptr && p->handler == h && p->what == what)
    {
        NMessage *n = p->next;
        message = n;
        delete p; // 注意：移除的节点，内存回收
        p = n;
    }

    // Remove all messages after front.
    // 链表中移除节点
    while (p != nullptr)
    {
        NMessage *n = p->next;
        if (n != nullptr)
        {
            if (n->handler == h && n->what == what)
            {
                // 断链：节点移除
                NMessage *nn = n->next;
                delete n; // 注意：移除的节点，内存回收
                p->next = nn;
                continue;
            }
        }
        p = n;
    }
}


void NMessageQueue::addIdleHandler(std::shared_ptr<NIdleHandler> handler)
{
    if (handler == nullptr)
    {
        return;
    }
    std::unique_lock<std::mutex> locker(mutex); // 注意上锁

    auto iter = std::find(idleHandlers.begin(), idleHandlers.end(), handler);
    if (iter != idleHandlers.end())
    {
        // 已经存在
        return;
    }
    idleHandlers.push_back(handler);
}

void NMessageQueue::removeIdleHandler(std::shared_ptr<NIdleHandler> handler)
{
    if (handler == nullptr)
    {
        return;
    }
    std::unique_lock<std::mutex> locker(mutex); // 注意上锁

    std::remove(idleHandlers.begin(), idleHandlers.end(), handler);
}

NMessage *NMessageQueue::next()
{
    // IO多路复用不生效了
    if (fd[0] <= 0)
    {
        return nullptr;
    }

    int pendingIdleHandlerCount = -1; // -1 only during first iteration
    int nextPollTimeoutMillis = 0;
    for (;;)
    {
        pollOnce(nextPollTimeoutMillis);

        std::unique_lock<std::mutex> locker(mutex); // 注意上锁

        long now = NSystemClock::uptimeMillis();
        NMessage *msg = message;

        if (msg != nullptr)
        {
            if (now < msg->when)
            {
                // Next message is not ready.  Set a timeout to wake up when it is ready.
                long timeoutMs = msg->when - now;
                nextPollTimeoutMillis = timeoutMs < INT_MAX ? timeoutMs : INT_MAX;
            } else
            {
                // Got a message.
                blocked = false;
                message = msg->next;
                msg->next = nullptr;
                return msg;
            }
        } else
        {
            // No more messages.
            nextPollTimeoutMillis = -1;
        }

        // Process the quit message now that all pending messages have been handled.
        if (quitting)
        {
            destroy();
            return nullptr;
        }

        // If first time idle, then get the number of idlers to run.
        // Idle handles only run if the queue is empty or if the first message
        // in the queue (possibly a barrier) is due to be handled in the future.
        if (pendingIdleHandlerCount < 0 && (message == nullptr || now < message->when))
        {
            pendingIdleHandlerCount = idleHandlers.size();
        }
        if (pendingIdleHandlerCount <= 0)
        {
            blocked = true;
            continue;
        }
        // 创建一个副本
        std::shared_ptr<NIdleHandler> pendingIdleHandlers[pendingIdleHandlerCount];
        for (int i = 0; i < pendingIdleHandlerCount; ++i)
        {
            pendingIdleHandlers[i] = idleHandlers[i];
        }

        // 提前放锁，给消息入队和出队等操作留出时间 enqueueMessage removeMessages...
        locker.unlock();

        // Run the idle handlers.
        // We only ever reach this code block during the first iteration.
        for (int i = 0; i < pendingIdleHandlerCount; ++i)
        {
            std::shared_ptr<NIdleHandler> idler = pendingIdleHandlers[i];

            bool keep = false;
            try
            {
                keep = idler->queueIdle();
            } catch (...)
            {
                // ignore
            }

            if (!keep)
            {
                std::unique_lock<std::mutex> idleLocker(mutex); // 注意上锁
                // 不需要保存的空闲任务，加锁移除
                std::remove(idleHandlers.begin(), idleHandlers.end(), idler);
            }
        }

        // Reset the idle handler count to 0 so we do not run them again.
        pendingIdleHandlerCount = 0;

        // While calling an idle handler, a new message could have been delivered
        // so go back and look again for a pending message without waiting.
        nextPollTimeoutMillis = 0;
    }

}

void NMessageQueue::quit()
{
    std::unique_lock<std::mutex> locker(mutex); // 注意上锁

    if (quitting)
    {
        return;
    }

    quitting = true;

    // 移除链表
    NMessage *p = message;
    while (p != nullptr)
    {
        NMessage *n = p->next;
        delete p;
        p = n;
    }
    message = nullptr;

    // 移除 IdleHandler
    idleHandlers.clear();

    wake();
}