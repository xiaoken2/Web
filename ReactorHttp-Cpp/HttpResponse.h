#pragma once
#include "Buffer.h"

// 定义状态码
enum HttpstatusCode {
    Unknown,
    OK = 200, 
    MovedPermanently = 301,
    MovedTemporal = 302,
    BadRequest = 400,
    NotFound = 404
};

// 定义键值对
struct ResponseHeader {
    char key[32];
    char value[32];
};

// 定义一个函数指针来组织要回复给客户端的数据块
typedef void (*responseBody)(const char* fileName, struct Buffer* sendBuf, int socket);

// 定义结构体
struct HttpResponse
{
    // 状态行：状态码，状态描述
    enum HttpstatusCode statuscode;
    char statusMsg[128];
    char fileName[128];
    // 响应头-键值对
    struct ResponseHeader* headers;
    int headNum;
    responseBody sendDataFunc;
};

// 初始化
struct HttpResponse* httpResponseInit();
// 销毁函数
void httpResponseDestroy(struct HttpResponse* response);
// 添加响应头
void httpResponseAddHeader(struct HttpResponse* response, const char* key, const char* value);
// 组织http响应函数
void httpResponsePrepareMsg(struct HttpResponse* response, struct Buffer* sendBuf, int socket);