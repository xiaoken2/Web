#include <stdio.h>
#include <unistd.h>

int main(int argc, char* argv[]) {
    if (argc < 3) {
        printf("./a.out port path \n");
        return -1;
    }

    unsigned short port = atoi(argv[1]);
    // 切换服务器的工作目录
    chdir(argv[2]);
    // 初始化用于监听的套接字
    int lfd = initListedFd(port);
    return 0;
}