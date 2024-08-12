#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "TcpServer.h"

int main(int argc, char* argv[]) {
    if (argc < 3) {
        printf("./a.out port path \n");
        return -1;
    }

    unsigned short port = atoi(argv[1]);
    // 切换服务器的工作目录
    chdir(argv[2]);
    // 启动服务器
    TcpServer* server = new TcpServer(port, 4);
    server->run();

    return 0;
}