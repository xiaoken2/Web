#include <stdio.h>
#include <unistd.h>
#include "Server.h"

int main(int agrv, char* agrc[]) {
    if (agrv < 3) {
        printf("./out port path \n");
        return -1;
    }
    unsigned short port = agrc[1];
    // 切换服务器的工作目录
    chdir(agrc[2]);
    // 初始化用于监听的套接字
    int lfd = initListedFd(port);
    // 启动epoll
    int epollRun(int lfd);

    return 0;
}