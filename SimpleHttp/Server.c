#include "Server.h"
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include <assert.h>
#include <sys/sendfile.h>
#include <dirent.h>
#include <unistd.h>
#include <stdlib.h>

int initListedFd(unsigned short port) {
    printf("init.... \n");
    int lfd = socket(AF_INET, SOCK_STREAM, 0);
    if (lfd == -1) {
        perror("socket");
        return -1;
    }

    int opt = 1;
    int ret = setsockopt(lfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof opt);
    if (ret == -1) {
        perror("setsockopt");
        return -1;
    }

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;

    ret = bind(lfd, (struct sockaddr*)&addr, sizeof addr);
    if (ret == -1) {
        perror("bind");
        return -1;
    }

    ret = listen(lfd, 128);
    if (ret == -1) {
        perror("listen");
        return -1;
    }

    return lfd;
}

int epollRun(int lfd) {
    printf("epollRun\n");
    int epfd = epoll_create(1);
    if (epfd == -1) {
        perror("epoll_create");
        return -1;
    }

    struct epoll_event ev;
    ev.data.fd = lfd;
    ev.events = EPOLLIN;

    int ret = epoll_ctl(epfd, EPOLL_CTL_ADD, lfd, &ev);
    if (ret == -1) {
        perror("epoll_ctl");
        return -1;
    }

    struct epoll_event evs[1024];
    int len = sizeof(evs) / sizeof(struct epoll_event);

    while (1) {
        int num = epoll_wait(epfd, evs, len, -1);
        for (int i = 0; i < num; i++) {
            int fd = evs[i].data.fd;
            if (fd == lfd) {
                acceptClinent(lfd, epfd);
            } else {
                recvHttpRequest(fd, epfd);
            }
        }
    }

    return 0;
}

int acceptClinent(int lfd, int epfd) {
    int cfd = accept(lfd, NULL, NULL);
    if (cfd == -1) {
        perror("accept");
        return -1;
    }

    int flag = fcntl(cfd, F_GETFL);
    flag |= O_NONBLOCK;
    fcntl(cfd, F_SETFL, flag);

    struct epoll_event ev;
    ev.data.fd = cfd;
    ev.events = EPOLLIN | EPOLLET;

    int ret = epoll_ctl(epfd, EPOLL_CTL_ADD, cfd, &ev);
    if (ret == -1) {
        perror("epoll_ctl");
        return -1;
    }

    return 0;
}

int recvHttpRequest(int cfd, int epfd) {
    printf("开始接受数据...\n");
    int len = 0;
    int total = 0;
    char tmp[1024] = {0};
    char buf[4096] = {0};

    while ((len = recv(cfd, tmp, sizeof tmp, 0)) > 0) {
        if (total + len < sizeof buf) {
            memcpy(buf + total, tmp, len);
        }
        total += len;
    }

    if (len == -1 && errno == EAGAIN) {
        char* pt = strstr(buf, "\r\n");
        if (pt != NULL) {
            int reqLen = pt - buf;
            buf[reqLen] = '\0';
            parseRequestLine(buf, cfd);
        }
    } else if (len == 0) {
        epoll_ctl(epfd, EPOLL_CTL_DEL, cfd, NULL);
        close(cfd);
    } else {
        perror("recv");
        return -1;
    }
    return 0;
}

int parseRequestLine(const char* line, int cfd) {
    char method[12];
    char path[1024];

    sscanf(line, "%[^ ] %[^ ]", method, path);
    printf("method: %s, path: %s\n", method, path);
    if (strcasecmp(method, "GET") != 0) {
        return -1;
    }

    char* file = NULL;
    if (strcmp(path, "/") == 0) {
        file = "./";
    } else {
        file = path + 1;
    }

    struct stat st;
    int ret = stat(file, &st);
    if (ret == -1) {
        sendHeadmsg(cfd, 404, "Not Found", getFileType(".html"), -1);
        sendFile("404.html", cfd);
    } else if (S_ISDIR(st.st_mode)) {
        sendHeadmsg(cfd, 200, "OK", getFileType(".html"), -1);
        sendDir(file, cfd);
    } else {
        sendHeadmsg(cfd, 200, "OK", getFileType(file), st.st_size);
        sendFile(file, cfd);
    }

    return 0;
}

const char* getFileType(const char* name) {
    const char* dot = strrchr(name, '.');
    if (dot == NULL)
        return "text/plain; charset=utf-8";
    if (strcmp(dot, ".html") == 0 || strcmp(dot, ".htm") == 0)
        return "text/html; charset=utf-8";
    if (strcmp(dot, ".jpg") == 0 || strcmp(dot, ".jpeg") == 0)
        return "image/jpeg";
    if (strcmp(dot, ".gif") == 0)
        return "image/gif";
    if (strcmp(dot, ".png") == 0)
        return "image/png";
    if (strcmp(dot, ".css") == 0)
        return "text/css";
    if (strcmp(dot, ".au") == 0)
        return "audio/basic";
    if (strcmp(dot, ".wav") == 0)
        return "audio/wav";
    if (strcmp(dot, ".avi") == 0)
        return "video/x-msvideo";
    if (strcmp(dot, ".mov") == 0 || strcmp(dot, ".qt") == 0)
        return "video/quicktime";
    if (strcmp(dot, ".mpeg") == 0 || strcmp(dot, ".mpe") == 0)
        return "video/mpeg";
    if (strcmp(dot, ".vrml") == 0 || strcmp(dot, ".wrl") == 0)
        return "model/vrml";
    if (strcmp(dot, ".midi") == 0 || strcmp(dot, ".mid") == 0)
        return "audio/midi";
    if (strcmp(dot, ".mp3") == 0)
        return "audio/mpeg";
    if (strcmp(dot, ".ogg") == 0)
        return "application/ogg";
    if (strcmp(dot, ".pac") == 0)
        return "application/x-ns-proxy-autoconfig";

    return "text/plain; charset=utf-8";
}

int sendDir(const char* dirName, int cfd) {
    char buf[4096] = {0};
    sprintf(buf, "<html><head><title>%s</title></head><body><table>", dirName);
    struct dirent** namelist;
    int num = scandir(dirName, &namelist, NULL, alphasort);
    for (int i = 0; i < num; ++i) {
        char* name = namelist[i]->d_name;
        struct stat st;
        char subPath[1024] = {0};
        sprintf(subPath, "%s/%s", dirName, name);
        stat(subPath, &st);

        if (S_ISDIR(st.st_mode)) {
            sprintf(buf + strlen(buf),
                "<tr><td><a href=\"%s/\">%s</a></td><td>%ld</td></tr>",
                name, name, st.st_size);
        } else {
            sprintf(buf + strlen(buf),
                "<tr><td><a href=\"%s\">%s</a></td><td>%ld</td></tr>",
                name, name, st.st_size);
        }
        send(cfd, buf, strlen(buf), 0);
        memset(buf, 0, sizeof(buf));
        free(namelist[i]);
    }
    sprintf(buf, "</table></body></html>");
    send(cfd, buf, strlen(buf), 0);
    free(namelist);
    return 0;
}

int sendFile(const char* fileName, int cfd) {
    int fd = open(fileName, O_RDONLY);
    assert(fd > 0);

    int size = lseek(fd, 0, SEEK_END);
    lseek(fd, 0, SEEK_SET);
    sendfile(cfd, fd, NULL, size);
    close(fd);
    return 0;
}

int sendHeadmsg(int cfd, int status, const char* descr, const char* type, int length) {
    char buf[4096] = {0};
    sprintf(buf, "HTTP/1.1 %d %s\r\n", status, descr);
    sprintf(buf + strlen(buf), "Content-Type: %s\r\n", type);
    sprintf(buf + strlen(buf), "Content-Length: %d\r\n\r\n", length);

    send(cfd, buf, strlen(buf), 0);
    return 0;
}

