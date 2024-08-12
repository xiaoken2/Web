#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include "ThreadPool.h"

ThreadPool::ThreadPool(EventLoop *mainLoop, int count)
{
    m_index = 0;
    m_isStart = false;
    m_mainLoop = mainLoop;
    m_threadNum = count;
    m_workerThreads.clear();
}

ThreadPool::~ThreadPool()
{
    for (auto item : m_workerThreads) {
        delete(item);
    }
}

void ThreadPool::run()
{
    // 判断主线程是不是没有运行
    assert(!m_isStart);
    // 判断是不是主线程
    if (m_mainLoop->getThreadID() != thread::id()) {
        exit(0);
    }

    m_isStart = true;
    if (m_threadNum > 0) {
        for (int i = 0; i < m_threadNum; ++i) {
            WorkerThread* subThread = new WorkerThread(i);
            subThread->run();
            m_workerThreads.push_back(subThread);
        }
    }
}

EventLoop *ThreadPool::takeWorkerEventLoop()
{
    // 判断主线程是不是在运行
    assert(m_isStart);
    // 判断是不是主线程
    if (m_mainLoop->getThreadID() != thread::id()) {
        exit(0);
    }

    // 从线程池中找到一个子线程，取出里面的反应堆模型
    EventLoop* evLoop = m_mainLoop;
    if (m_threadNum) {
        evLoop = m_workerThreads[m_index]->getEventLoop();
        m_index = ++m_index % m_threadNum;
    }

    return evLoop;
    return nullptr;
}
