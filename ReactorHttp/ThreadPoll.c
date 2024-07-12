#include <stdio.h>
#include <assert.h>
#include "ThreadPoll.h"

struct ThreadPoll* threadPollInit(struct EventLoop* mainLoop, int count) {
    struct ThreadPoll* poll = (struct ThreadPoll*)malloc(sizeof(struct ThreadPoll));
    poll->index = 0;
    poll->isStart = false;
    poll->mainLoop = mainLoop;
    poll->threadNum = count;
    poll->workerThread = (struct WorkerThread*)malloc(sizeof(struct WorkerThread*)*count);
    return poll;
}

void threadPollRun(struct ThreadPoll* pool) {
    // 判断主线程是不是没有运行
    assert(pool && !pool->isStart);

    // 判断是不是主线程
    if (pool->mainLoop->threadID != pthread_self()) {
        exit(0);
    }

    pool->isStart = true;
    if (pool->threadNum) {
        for (int i = 0; i < pool->threadNum; ++i) {
            workerThreadInit(&pool->workerThread[i], i);
            workerThreadrun(&pool->workerThread[i]);
        }
    }
}

struct EventLoop* takeWorkerEventLoop(struct ThreadPoll* pool) {
    // 判断主线程是不是在运行
    assert(pool->isStart);
    // 判断是不是主线程
    if (pool->mainLoop->threadID != pthread_self()) {
        exit(0);
    }

    // 从线程池中找到一个子线程，取出里面的反应堆模型
    struct EventLoop* evLoop = pool->mainLoop;
    if (pool->threadNum) {
        evLoop = pool->workerThread[pool->index].evLoop;
        pool->index = ++pool->index % pool->threadNum;
    }

    return evLoop;
}

