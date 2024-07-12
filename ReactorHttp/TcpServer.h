#pragma once
#include "EventLoop.h"
#include "ThreadPoll.h"

struct Listener {
    int lfd;
    unsigned short port;
};

struct TcpServer
{
    int ThreadNum;
    struct EventLoop* mainLoop;
    struct ThreadPoll*  threadPool;
    struct Listener* listener;
};

// 初始化
struct TcpServer* tcpServerInit(unsigned short port, int ThreadNum);
// 初始化监听
struct Listener* listenerInit(unsigned short port);
// 启动服务器
void tcpServerRun(struct TcpServer* server);
