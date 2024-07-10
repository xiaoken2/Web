#pragma once


// 初始化用于监听的套接字
int initListedFd(unsigned short port);
// 启动epoll
int epollRun(int lfd);

// 和客户端建立连接
int acceptClinent(int lfd, int epfd);



