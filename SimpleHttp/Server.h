#pragma once


// 初始化用于监听的套接字
int initListedFd(unsigned short port);
// 启动epoll
int epollRun(int lfd);

// 和客户端建立连接
// int acceptClinent(int lfd, int epfd);
void* acceptClinent(void* arg);

// 接收http请求消息
// int recvHttpRequest(int cfd, int epfd);
void* recvHttpRequest(void* arg);

// 解析请求行
int parseRequestLine(const char* line, int cfd);

// 发送文件
int sendFile (const char* fileName, int cfd);

// 发送响应头（状态行和响应行）
int sendHeadmsg(int cfd, int status, const char* descr, const char* type, int length);

// 根据文件名来获取content-type
const char* getFileType(const char* name);

// 发送目录
int sendDir(const char* dirName, int cfd);

// 编码来访问中文名字和特殊字符
int hexToDec(char c);
void decodeMsg(char* to, char* from);
