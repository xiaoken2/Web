#include <arpa/inet.h>
#include <stdlib.h>
#include <stdio.h>
#include "TcpServer.h"
#include "TcpConnection.h"
#include "Log.h"

TcpServer::TcpServer(unsigned short port, int threadNum)
{
    m_port = port;
    m_threadNum = threadNum;
    m_mainLoop = new EventLoop;
    m_threadPool = new ThreadPool(m_mainLoop, threadNum);;
    setLlistener();

}


void TcpServer::setLlistener()
{
    m_lfd = socket(AF_INET, SOCK_STREAM, 0); // IPV4的流式协议（TCP）
    if(m_lfd == -1){
        perror("socker");
        return;
    }
    // 2. 设置端口复用
    int opt = 1;
    int ret = setsockopt(m_lfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof opt);
    if (ret == -1) {
        perror("setsockopt");
        return;
    }
    // 3. 绑定端口
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;  // ipv4协议
    addr.sin_port = htons(m_port);  // 绑定的端口, 把端口从小端转换成大端
    addr.sin_addr.s_addr = INADDR_ANY;  // 绑定的ip地址（0地址）

    ret = bind(m_lfd, (struct sockaddr*)&addr, sizeof addr);

    if (ret == -1) {
        perror("bind");
        return;
    }
    // 4. 设置监听
    ret = listen(m_lfd, 128);
    if (ret == -1) {
        perror("listen");
        return;
    }
}

void TcpServer::run()
{
    Debug("程序已经启动了...");
    // 启动线程池
    m_threadPool->run();
    // 添加检测的任务
    // 初始化一个channel实例
    Channel* channel = new Channel(m_lfd, FDEvent::ReadEvent, acceptConnect, nullptr, nullptr, this);
    m_mainLoop->addTask(channel, ElemType::ADD);
    // 启动反应堆模型
    m_mainLoop->run();
}

int TcpServer::acceptConnect(void *arg)
{
    TcpServer* server = static_cast<TcpServer*>(arg);
    // 和客户端建立连接
    int cfd = accept(server->m_lfd, nullptr, nullptr);  // 第二参数为连接的客户端的端口和ip地址，不需要这些信息的时候为NULL，第三个参数为第二个参数的结构体大小
    // 从线程池里面取出子线程的反应堆实例
    EventLoop* evLoop = server->m_threadPool->takeWorkerEventLoop();

    // 将cfd放到TcpConnection中处理
    new TcpConnection(cfd, evLoop);
    return 0;
}
