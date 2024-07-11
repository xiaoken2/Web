#include "Dispatcher.h"
#include <poll.h>

#define Max 1024

struct PollData
{
    int maxfd;
    struct pollfd fds[Max];
};


static void* pollInit();
// 添加
static int pollAdd (struct Channel* channel, struct EventLoop* evLoop); //通过EventLoop* evLoop取到不同的epoll,poll或者select需要的数据块
// 删除
static int pollRemove (struct Channel* channel, struct EventLoop* evLoop);
// 修改
static int pollModify (struct Channel* channel, struct EventLoop* evLoop);
// 事件监测
static int pollDispatch (struct EventLoop* evLoop, int timeout); // timeout单位 s
// 清除数据
static int pollClear (struct EventLoop* evLoop);
 

struct Dispatcher PollDispatcher = {
    pollInit,
    pollAdd,
    pollModify,
    pollDispatch,
    pollClear
};

static void* pollInit() {
    struct PollData* data = (struct PollData*)malloc(sizeof(struct PollData));
    data->maxfd = 0;
    for (int i = 0; i < Max; ++i) {
        data->fds[i].fd = -1;
        data->fds[i].events = 0;
        data->fds[i].revents = 0;
    }

    return data;

}

static int pollAdd (struct Channel* channel, struct EventLoop* evLoop) {
    
    struct PollData* data = (struct PollData*)evLoop->dispatcherData;
    int events = 0;
    if (channel->events & ReadEvent) {
        events |= POLLIN;
    }

    if (channel->events & WriteEvent) {
        events |= POLLOUT;
    }

    int i = 0;
    for (; i < Max; i++) {
        if (data->fds[i].fd == -1) {
            data->fds[i].events = events;
            data->fds[i].fd = channel->fd;
            data->maxfd = i > data->maxfd ? i : data->maxfd;
            break;
        }
    }
    if (i >= Max) {
        return -1;
    }
    return 0;
}

static int pollRemove (struct Channel* channel, struct EventLoop* evLoop) {
    struct PollData* data = (struct PollData*)evLoop->dispatcherData;
    int events = 0;

    int i = 0;
    for (; i < Max; i++) {
        if (data->fds[i].fd == channel->fd) {
            data->fds[i].events = 0;
            data->fds[i].revents = 0;
            data->fds[i].fd = -1;
            break;
        }
    }
    if (i >= Max) {
        return -1;
    }
    return 0;
}

static int pollModify (struct Channel* channel, struct EventLoop* evLoop) {
    struct PollData* data = (struct PollData*)evLoop->dispatcherData;
    int events = 0;
    if (channel->events & ReadEvent) {
        events |= POLLIN;
    }

    if (channel->events & WriteEvent) {
        events |= POLLOUT;
    }

    int i = 0;
    for (; i < Max; i++) {
        if (data->fds[i].fd == channel->fd) {
            data->fds[i].events = events;
            break;
        }
    }
    if (i >= Max) {
        return -1;
    }
    return 0;
}

static int pollDispatch (struct EventLoop* evLoop, int timeout) { // timeout单位 s
    struct PollData* data = (struct PollData*)evLoop->dispatcherData;
    int count = epoll_wait(data->fds, data->maxfd + 1, Max, timeout * 1000);
    if (count == -1) {
        perror("poll");
        exit(0);
    }
    for (int i = 0; i <= data->maxfd; ++i) {
        if (data->fds[i].fd == -1) {
            continue;
        }
        if (data->fds[i].revents & POLLIN) {
            // 读事件触发

        }
        if (data->fds[i].revents & POLLOUT) {
            // 写事件
        }
    }
    return 0;
} 
static int pollClear (struct EventLoop* evLoop) {
    struct PollData* data = (struct PollData*)evLoop->dispatcherData;
    free(data);
}