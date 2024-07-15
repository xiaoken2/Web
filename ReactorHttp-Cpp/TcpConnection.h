#pragma once
#include "EventLoop.h"
#include "Channel.h"
#include "Buffer.h"
#include "HttpRequest.h"
#include "HttpResponse.h"

// #define MSG_SEND_AUTO

struct TcpConnection
{
    struct EventLoop* evLoop;
    struct Channel* channel;
    struct Buffer* readBuf;
    struct Buffer* writeBuf;
    char name[32];
    // http
    struct HttpRequest* request;
    struct HttpResponse* response;
};

// 初始化
struct TcpConnection* tcpConnectionInit(int fd, struct EventLoop* evLoop);

// 资源释放函数
int tcpConnectionDestroy(void*);
