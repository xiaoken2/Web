#include "Server.h"
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <stdio.h>


int initListedFd(unsigned short port) {
    // 1. 创建监听的fd
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
    addr.sin_port = htons(port);  // 绑定的端口
    addr.sin_addr.s_addr = INADDR_ANY;  // 绑定的ip地址

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
    return lfd;
}

int epollRun(int lfd) {
    int epfd = epoll_create(1);
    if (epfd == -1) {
        perror("epoll_create");
        return -1;
    }

    // 2. lfd 上树
    struct epoll_event ev;
    ev.data.fd = lfd;
    ev.events = EPOLLIN;
    
    int ret = epoll_ctl(epfd, EPOLL_CTL_ADD, lfd, &ev);
    if (ret == -1) {
        perror("epoll_ctl");
        return -1;
    }

    // 检测
    struct epoll_event evs[1024];
    
    while (1) {
        int num = epoll_wait(epfd, evs, sizeof evs, -1);
        for (int i = 0; i < num; i++) {
            int fd = evs[i].data.fd;
            if (fd = lfd) {
                // 用于监听的套接字，建立连接 accept
                acceptClinent(lfd, epfd);
            } else {
                // 用于通信的套接字，接收对端的数据
            }
        }
    }
    

    return 0;
}


int acceptClinent(int lfd, int epfd) {
    // 1. 建立连接
    int cfd = accept(lfd, NULL, NULL);
    if (cfd == -1) {
        perror("accept");
        return -1;
    }

    // 2. 设置非阻塞, epoll的工作模式是边沿非阻塞
    int flag = fcntl1(cfd, F_GETFL);
    flag |= O_NONBLOCK;
    fcntl(cfd, F_SETFL, flag);

    // 3. cfd添加到epoll中
    struct epoll_event ev;
    ev.data.fd = cfd;
    ev.events = EPOLLIN | EPOLLET;
    
    int ret = epoll_ctl(epfd, EPOLL_CTL_ADD, cfd, &ev);
    if (ret == -1) {
        perror("epoll_ctl");
        return -1;
    }

}



