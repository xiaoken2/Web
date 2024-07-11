#include <assert.h>

#include "EventLoop.h"


struct EventLoop* eventLoopInit() {
    return eventLoopInitEx(NULL);
}

struct EventLoop* eventLoopInitEx(const char* threadName) {
    struct EventLoop* evLoop = (struct EventLoop*)malloc(sizeof(struct EventLoop));
    evLoop->isQuit = false;
    evLoop->threadID = pthread_self();
    pthread_mutex_init(&evLoop->mutex, NULL);
    strcpy(evLoop->threadName, threadName == NULL ? "MainThread" : threadName);
    evLoop->dispatcher = &EpollDispatcher;
    evLoop->dispatcherData = evLoop->dispatcher->init();
    // 初始化链表
    evLoop->head = evLoop->tail = NULL;
    // 初始化map
    evLoop->channelMap = channelMapInit(128);
    return evLoop;
}

int eventLoopRun(struct EventLoop* evLoop) {
    assert(evLoop != NULL);
    // 取出事件分发和检测模型
    struct Dispatcher* dispatcher = evLoop->dispatcher;
    // 比较线程ID是否正常
    if (evLoop->threadID != pthread_self) {
        return -1;
    }
    // 循环进行事件处理
    while (!evLoop->isQuit) {
        dispatcher->dispatch(evLoop, 2); // 2是超时时长
    }
    return 0;
}

int eventActivate(struct EventLoop* evLoop, int fd, int event) {
    if (fd < 0 || evLoop == NULL) {
        return -1;
    }

    struct Channel* channel = evLoop->channelMap->list[fd];
    assert(channel->fd == fd);

    if (event & ReadEvent && channel->readCallback) {
        channel->readCallback(channel->arg);
    }

    if (event & WriteEvent && channel->writeCallback) {
        channel->writeCallback(channel->arg);
    }

    return 0;
}

