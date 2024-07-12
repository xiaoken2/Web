#pragma once
#include "EventLoop.h"
#include "WorkerThread.h"

// 定义线程池的结构体
struct ThreadPoll
{
    struct EventLoop* mainLoop;
    bool isStart;
    int threadNum;
    struct WorkerThread* workerThread;
    int index;
};

// 初始化线程池
struct ThreadPoll* threadPollInit(struct EventLoop* mainLoop, int threadNum);

// 启动线程池
void threadPollRun(struct ThreadPoll* pool);

// 取出线程池中的某个子线程
struct EventLoop* takeWorkerEventLoop(struct ThreadPoll* pool);
