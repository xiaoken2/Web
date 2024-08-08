#pragma once
#include "EventLoop.h"
#include "ThreadPool.h"

struct Listener {

};

class TcpServer
{
public:
    // 初始化
    TcpServer(unsigned short port, int ThreadNum);
    // ~TcpServer();
    // 初始化监听
    void setLlistener();
    // 启动服务器
    void run();
    static int acceptConnect(void* arg);
private:
    int m_threadNum;
    EventLoop* m_mainLoop;
    ThreadPool*  m_threadPool;
    int m_lfd;
    unsigned short m_port;
};


