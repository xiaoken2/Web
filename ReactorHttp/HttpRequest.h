#pragma once

// 请求的键值对
struct RequestHeader{
    char key;
    char value;
};

// 当前的解析状态
enum HttpRequestState {
    ParseReqLine,
    ParseReqHeaders,
    ParseReqBody,
    ParseReqDone
};

// 定义http请求结构体
struct HttpRequest
{
    char* method;
    char* url;
    char* version;
    struct RequestHeader* reqHeaders;
    int reqHeadersNum;
    enum HttpRequestState curState;
};

// 初始化
struct HttpRequest* httpRequestInit();

// 重置HttpRequest结构体
void httpRequestReset(struct HttpRequest* req);
void httpRequestResetEx(struct HttpRequest* req);
// 添加内存释放函数
void httpRequestDestroy(struct HttpRequest* req);

// 获取处理状态的函数
enum HttpRequestState HttpRequestState(struct HttpRequest* request);

// 添加请求头


// 根据key得到请求头的value






