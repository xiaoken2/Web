#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#include "Server.h"

int main(int agrc, char* agrv[]) {
    if (agrc < 3) {
        printf("./a.out port path \n");
        return -1;
    }
    unsigned short port = atoi(agrv[1]);
    // 切换服务器的工作目录
    chdir(agrv[2]);
    // 初始化用于监听的套接字
    int lfd = initListedFd(port);
    // 启动epoll
    epollRun(lfd);
    printf("44444\n");

    return 0;
}
