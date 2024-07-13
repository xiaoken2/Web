#include "Dispatcher.h"
#include <sys/select.h>
#include <stdio.h>

#define Max 1024

struct SelectData
{
    fd_set readSet;
    fd_set writSet;
};


static void* selectInit();
// 添加
static int selectAdd (struct Channel* channel, struct EventLoop* evLoop); //通过EventLoop* evLoop取到不同的epoll,poll或者select需要的数据块
// 删除
static int selectRemove (struct Channel* channel, struct EventLoop* evLoop);
// 修改
static int selectModify (struct Channel* channel, struct EventLoop* evLoop);
// 事件监测
static int selectDispatch (struct EventLoop* evLoop, int timeout); // timeout单位 s
// 清除数据
static int selectClear (struct EventLoop* evLoop);

static void setFdSet(struct Channel* channel, struct SelectData* data);
static void clearFdSet(struct Channel* channel, struct SelectData* data);
 

struct Dispatcher SelectDispatcher = {
    selectInit,
    selectAdd,
    selectModify,
    selectDispatch,
    selectClear
};

static void* selectInit() {
    struct SelectData* data = (struct SelectData*)malloc(sizeof(struct SelectData));
    FD_ZERO(&data->readSet);
    FD_ZERO(&data->writSet);

    return data;

}

static void setFdSet(struct Channel* channel, struct SelectData* data) {
    if (channel->events & ReadEvent) {
        FD_SET(channel->fd, &data->readSet);
    }

    if (channel->events & WriteEvent) {
        FD_SET(channel->fd, &data->writSet);
    }
}

static void clearFdSet(struct Channel* channel, struct SelectData* data) {
    if (channel->events & ReadEvent) {
        FD_CLR(channel->fd, &data->readSet);
    }

    if (channel->events & WriteEvent) {
        FD_CLR(channel->fd, &data->writSet);
    }
}

static int selectAdd (struct Channel* channel, struct EventLoop* evLoop) {
    
    struct SelectData* data = (struct SelectData*)evLoop->dispatcherData;
    if (channel->fd >= Max) {
        return -1;
    }
    setFdSet(channel, data);

    return 0;
}

static int selectRemove (struct Channel* channel, struct EventLoop* evLoop) {
    struct SelectData* data = (struct SelectData*)evLoop->dispatcherData;
    clearFdSet(channel, data);
    // 通过channel释放对应的TcpConnection资源
    channel->destroyCallback(channel->arg);
    return 0;
}

static int selectModify (struct Channel* channel, struct EventLoop* evLoop) {
    struct SelectData* data = (struct SelectData*)evLoop->dispatcherData;
    clearFdSet(channel, data);
    setFdSet(channel, data);

    return 0;
}

static int selectDispatch (struct EventLoop* evLoop, int timeout) { // timeout单位 s
    struct SelectData* data = (struct SelectData*)evLoop->dispatcherData;
    struct timeval val;
    val.tv_sec = timeout;
    val.tv_usec = 0;
    fd_set rdtmp = data->readSet;
    fd_set wrtmp = data->writSet;
    int count = select(Max, &rdtmp, &wrtmp, NULL, &val);
    if (count == -1) {
        perror("select");
        exit(0);
    }
    for (int i = 0; i < Max; ++i) {
        if (FD_ISSET(i, &rdtmp)) {
            // 读事件
            eventActivate(evLoop, i, ReadEvent);
        }
        if (FD_ISSET(i, &wrtmp)) {
            // 写事件触发
            eventActivate(evLoop, i, WriteEvent);
        }
    }
    return 0;
} 
static int selectClear (struct EventLoop* evLoop) {
    struct SelectData* data = (struct SelectData*)evLoop->dispatcherData;
    free(data);
}