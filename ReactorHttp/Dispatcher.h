#pragma once
#include "EventLoop.h"
#include "Channel.h"
struct EventLoop;
struct Dispatcher
{
    // init -- 初始化epoll,poll或者select需要的数据块
    void (*init)();
    // 添加
    int (*add)(struct Channel* channel, struct EventLoop* evLoop); //通过EventLoop* evLoop取到不同的epoll,poll或者select需要的数据块
    // 删除
    int (*remove)(struct Channel* channel, struct EventLoop* evLoop);
    // 修改
    int (*modify)(struct Channel* channel, struct EventLoop* evLoop);
    // 事件监测
    int (*dispatch)(struct EventLoop* evLoop, int timeout); // timeout单位 s
    // 清除数据
    int (*clear)(struct EventLoop* evLoop);
};
