#include <assert.h>
#include <sys/socket.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

#include "EventLoop.h"


struct EventLoop* eventLoopInit() {
    return eventLoopInitEx(NULL);
}

// 往socketPair中写数据的函数
void taskWakeup(struct EventLoop* evLoop) {
    const char* msg = "ddddd";
    write(evLoop->socketPair[0], msg, strlen(msg));
}

int readLocalMessage(void* arg) {
    struct EventLoop* evLoop = (struct EventLoop*)arg;
    char buf[256];
    read(evLoop->socketPair[1], buf, sizeof(buf));
    return 0;
}

struct EventLoop* eventLoopInitEx(const char* threadName) {
    struct EventLoop* evLoop = (struct EventLoop*)malloc(sizeof(struct EventLoop));
    evLoop->isQuit = false;
    evLoop->threadID = pthread_self();
    pthread_mutex_init(&evLoop->mutex, NULL);
    strcpy(evLoop->threadName, threadName == NULL ? "MainThread" : threadName);
    evLoop->dispatcher = &SelectDispatcher;
    evLoop->dispatcherData = evLoop->dispatcher->init();
    // 初始化链表
    evLoop->head = evLoop->tail = NULL;
    // 初始化map
    evLoop->channelMap = channelMapInit(128);

    // 添加本地文件描述符的初始化
    int ret = socketpair(AF_UNIX, SOCK_STREAM, 0, evLoop->socketPair);
    if (ret == -1) {
        perror("socketpair");
        exit(0);
    }
    // 指定规则:evLoop->sockPair[0]为发送数据， evLoop->sockPair[1]为接收数据
    struct Channel* channel = channelInit(evLoop->socketPair[1], ReadEvent, readLocalMessage, NULL, NULL, evLoop);

    //将channel添加到任务队列中
    eventLoopAddTask(evLoop, channel, ADD);

    return evLoop;
}

int eventLoopRun(struct EventLoop* evLoop) {
    assert(evLoop != NULL);
    // 取出事件分发和检测模型
    struct Dispatcher* dispatcher = evLoop->dispatcher;
    // 比较线程ID是否正常
    if (evLoop->threadID != pthread_self()) {
        return -1;
    }
    // 循环进行事件处理
    while (!evLoop->isQuit) {
        dispatcher->dispatch(evLoop, 2); // 2是超时时长
        eventLoopProcessTask(evLoop);
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

int eventLoopAddTask(struct EventLoop* evLoop, struct Channel* channel, int type) {
    // 加锁保护共享资源
    pthread_mutex_lock(&evLoop->mutex);
    // 创建新的节点
    struct ChannelElement* node = (struct ChannelElement*)malloc(sizeof(struct ChannelElement));
    node->channel = channel;
    node->next = NULL;
    node->type = type;
    // 链表为空
    if (evLoop->head == NULL) {
        evLoop->head = evLoop->tail = node;
    }
    //链表不为空
    else {
        evLoop->tail->next = node;
        evLoop->tail = node;
    }
    pthread_mutex_unlock(&evLoop->mutex);

    // 处理节点
    /*
    1. 对于链表节点的添加：可能是当前线程（子线程）也可能是主线程
        1). 修改fd的事件,当前子线程发起,当前子线程处理
        2). 添加新的fd事件.添加任务节点的操作是由主线程发起的
    2. 不能让主线程处理任务队列,需要由当前的子线程处理
    */
    if (evLoop->threadID == pthread_self()) {
        // 当前子线程
        eventLoopProcessTask(evLoop);

    } else {
        // 主线程->告诉子线程处理任务队列中的任务
        // 1. 子线程在工作 2.子线程被阻塞了（Select，poll，epoll）
        taskWakeup(evLoop);
    }
}

int eventLoopProcessTask(struct EventLoop* evLoop) {
    pthread_mutex_lock(&evLoop->mutex);
    // 取出头结点
    struct ChannelElement* head = evLoop->head;
    while (head) {
        struct Channel* channel = head->channel;
        if (head->type == ADD) {
            // 添加
            eventLoopAdd(evLoop, channel);

        } else if(head->type == DELETE) {
            // 删除
            eventLoopRemove(evLoop, channel);
        } else if(head->type == MODIFY){
            // 修改
            eventLoopModify(evLoop, channel);
        }
        struct ChannelElement* tmp = head;
        head = head->next;
        free(tmp);
    }
    evLoop->head = evLoop->tail = NULL;
    pthread_mutex_unlock(&evLoop->mutex);

    return 0;
}

int eventLoopAdd(struct EventLoop* evLoop, struct Channel* channel) {
    int fd = channel->fd;
    struct ChannelMap* channelMap = evLoop->channelMap;

    if (fd >= channelMap->size) {
        // 没有足够的空间键值对，因此需要扩容
        if (!makeMapRoom(channelMap, fd, sizeof(struct Channel*))) {
            return -1;
        }
    }
    // 找到fd对应的数组元素位置，并存储
    if (channelMap->list[fd] == NULL) {
        channelMap->list[fd] = channel;
        evLoop->dispatcher->add(channel, evLoop);
    }
    return 0;
}


int eventLoopRemove(struct EventLoop* evLoop, struct Channel* channel) {
    int fd = channel->fd;
    struct ChannelMap* channelMap = evLoop->channelMap;

    if (fd >= channelMap->size) {
        return -1;
    }
    int ret = evLoop->dispatcher->remove(channel, evLoop);
    return ret;
}

int eventLoopModify(struct EventLoop* evLoop, struct Channel* channel) {
    int fd = channel->fd;
    struct ChannelMap* channelMap = evLoop->channelMap;

    if (fd >= channelMap->size) {
        return -1;
    }
    int ret = evLoop->dispatcher->modify(channel, evLoop);
    return ret;
}


int destroyChannel(struct EventLoop* evLoop, struct Channel* channel) {
    // 删除channel和fd的对应关系
    evLoop->channelMap->list[channel->fd] = NULL;
    // 关闭fd
    close(channel->fd);
    // 释放channel
    free(channel);
    return 0;
}
