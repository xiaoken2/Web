#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include "ThreadPool.h"

struct ThreadPool* threadPoolInit(struct EventLoop* mainLoop, int count) {
    struct ThreadPool* pool = (struct ThreadPool*)malloc(sizeof(struct ThreadPool));
    pool->index = 0;
    pool->isStart = false;
    pool->mainLoop = mainLoop;
    pool->threadNum = count;
    pool->workerThreads = (struct WorkerThread*)malloc(sizeof(struct WorkerThread)*count);
    return pool;
}

void threadPoolRun(struct ThreadPool* pool) {
    // 判断主线程是不是没有运行
    assert(pool && !pool->isStart);
    // 判断是不是主线程
    if (pool->mainLoop->threadID != pthread_self()) {
        exit(0);
    }

    pool->isStart = true;
    if (pool->threadNum) {
        for (int i = 0; i < pool->threadNum; ++i) {
            workerThreadInit(&pool->workerThreads[i], i);
            workerThreadRun(&pool->workerThreads[i]);
        }
    }
}

struct EventLoop* takeWorkerEventLoop(struct ThreadPool* pool) {
    // 判断主线程是不是在运行
    assert(pool->isStart);
    // 判断是不是主线程
    if (pool->mainLoop->threadID != pthread_self()) {
        exit(0);
    }

    // 从线程池中找到一个子线程，取出里面的反应堆模型
    struct EventLoop* evLoop = pool->mainLoop;
    if (pool->threadNum) {
        evLoop = pool->workerThreads[pool->index].evLoop;
        pool->index = ++pool->index % pool->threadNum;
    }

    return evLoop;
}

// #include "ThreadPool.h"
// #include <assert.h>
// #include <stdlib.h>

// struct ThreadPool* threadPoolInit(struct EventLoop* mainLoop, int count)
// {
//     struct ThreadPool* pool = (struct ThreadPool*)malloc(sizeof(struct ThreadPool));
//     pool->index = 0;
//     pool->isStart = false;
//     pool->mainLoop = mainLoop;
//     pool->threadNum = count;
//     pool->workerThreads = (struct WorkerThread*)malloc(sizeof(struct WorkerThread) * count);
//     return pool;
// }

// void threadPoolRun(struct ThreadPool* pool)
// {
//     assert(pool && !pool->isStart);
//     if (pool->mainLoop->threadID != pthread_self())
//     {
//         exit(0);
//     }
//     pool->isStart = true;
//     if (pool->threadNum)
//     {
//         for (int i = 0; i < pool->threadNum; ++i)
//         {
//             workerThreadInit(&pool->workerThreads[i], i);
//             workerThreadRun(&pool->workerThreads[i]);
//         }
//     }
// }

// struct EventLoop* takeWorkerEventLoop(struct ThreadPool* pool)
// {
//     assert(pool->isStart);
//     if (pool->mainLoop->threadID != pthread_self())
//     {
//         exit(0);
//     }
//     // 从线程池中找一个子线程, 然后取出里边的反应堆实例
//     struct EventLoop* evLoop = pool->mainLoop;
//     if (pool->threadNum > 0)
//     {
//         evLoop = pool->workerThreads[pool->index].evLoop;
//         pool->index = ++pool->index % pool->threadNum;
//     }
//     return evLoop;
// }

