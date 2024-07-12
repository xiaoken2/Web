#include <arpa/inet.h>
#include "TcpServer.h"
#include "TcpConnection.h"

struct TcpServer* tcpServerInit(unsigned short port, int threadNum) {
    struct TcpServer* tcp = (struct TcpServer*)malloc(sizeof(struct TcpServer));
    tcp->listener = listenerInit(port);
    tcp->mainLoop = eventLoopInit();
    tcp->ThreadNum = threadNum;
    tcp->threadPool = threadPollInit(tcp->mainLoop, threadNum);

    return tcp;
}

struct Listener* listenerInit(unsigned short port) {
    // 1. 创建监听的fd
    struct Listener* listener = (struct Listener*)malloc(sizeof(struct Listener));
    int lfd = socket(AF_INET, SOCK_STREAM, 0); // IPV4的流式协议（TCP）
    if(lfd == -1){
        perror("socker");
        return -1;
    }
    // 2. 设置端口复用
    int opt = 1;
    int ret = setsockopt(lfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof opt);
    if (ret == -1) {
        perror("setsockopt");
        return -1;
    }
    // 3. 绑定端口
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;  // ipv4协议
    addr.sin_port = htons(port);  // 绑定的端口, 把端口从小端转换成大端
    addr.sin_addr.s_addr = INADDR_ANY;  // 绑定的ip地址（0地址）

    ret = bind(lfd, (struct sockaddr*)&addr, sizeof addr);

    if (ret == -1) {
        perror("bind");
        return -1;
    }
    // 4. 设置监听
    ret = listen(lfd, 128);
    if (ret == -1) {
        perror("listen");
        return -1;
    }
    listener->lfd = lfd;
    listener->port = port;
    return listener;
}

int acceptConnect(void* arg) {
    struct TcpServer* server = (struct TcpServer*)arg;
    // 和客户端建立连接
    int cfd = accept(server->listener->lfd, NULL, NULL);  // 第二参数为连接的客户端的端口和ip地址，不需要这些信息的时候为NULL，第三个参数为第二个参数的结构体大小
    // 从线程池里面取出子线程的反应堆实例
    struct EventLoop* evLoop =  takeWorkerEventLoop(server->threadPool);

    // 将cfd放到TcpConnection中处理
    tcpConnectionInit(cfd, evLoop);

}

void tcpServerRun(struct TcpServer* server) {
    // 启动线程池
    threadPollRun(server->threadPool);
    // 添加检测的任务
    // 初始化一个channel实例
    struct Channel* channel = channelInit(server->listener->lfd, ReadEvent, acceptConnect, NULL, server);
    eventLoopAddTask(server->mainLoop, channel, ADD);
    // 启动反应堆模型
    eventLoopRun(server->mainLoop);

}
