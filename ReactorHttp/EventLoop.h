#pragma once
#include <stdbool.h>
#include <pthread.h>

#include "Dispatcher.h"
#include "ChannelMap.h"

extern struct Dispatcher EpollDispatcher;
extern struct Dispatcher PollDispatcher;
extern struct Dispatcher SelectDispatcher;

// 处理节点中的channel
enum ElemType{ADD, DELETE, MODIFY};

// 定义任务节点
struct ChannelElement
{
    int type;  // 如何处理节点中的channel
    struct Channel* channel;
    struct ChannelElement* next;
};

struct Dispatcher;
struct EventLoop{
    bool isQuit;
    struct Dispatcher* dispatcher;
    void* dispatcherData;
    // 任务队列
    struct ChannelElement* head;
    struct ChannelElement* tail;

    // map来映射文件描述符和channel之间的关系
    struct  ChannelMap* channelMap;

    // 线程id，name
    pthread_t threadID;
    char threadName[32];
    pthread_mutex_t mutex;

    // 用于唤醒阻塞中的子线程
    // 用于存储初始化的两个文件描述符
    int socketPair[2];
    
};

// 初始化
struct EventLoop* eventLoopInit();

struct EventLoop* eventLoopInitEx(const char* threadName);

// 启动反应堆模型
int eventLoopRun(struct EventLoop* evLoop);

// 处理激活的文件fd
int eventActivate(struct EventLoop* evLoop, int fd, int event);

// 添加任务到任务队列
int eventLoopAddTask(struct EventLoop* evLoop, struct Channel* channel, int type);

// 处理任务队列中的任务
int eventLoopProcessTask(struct EventLoop* evLoop);

// 处理dispatcher中的节点
int eventLoopAdd(struct EventLoop* evLoop, struct Channel* channel);
int eventLoopRemove(struct EventLoop* evLoop, struct Channel* channel);
int eventLoopModify(struct EventLoop* evLoop, struct Channel* channel);

// 删除节点以后需要释放channel资源
int destroyChannel(struct EventLoop* evLoop, struct Channel* channel);