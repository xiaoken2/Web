#pragma once
#include <stdbool.h>
#include <sys/types.h>
#include "HttpResponse.h"
#include "Buffer.h"
#include "TcpConnection.h"


// 请求的键值对
struct RequestHeader{
    char* key;
    char* value;
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
void HttpRequestAddHeader(struct HttpRequest* request, const char* key, const char* value);

// 根据key得到请求头的value
char HttpRequestGetHeader(struct HttpRequest* request, const char* key);

// 解析请求行
bool parseHttpRequestLine(struct HttpRequest* request, struct Buffer* readBuf);

// 解析请求头
bool parseHttpRequestHeader(struct HttpRequest* request, struct Buffer* readBuf);

// 解析http请求协议
bool parseHttpRequest(struct HttpRequest* request, struct Buffer* readBuf,
struct HttpResponse* response, struct Buffer* sendBuf, int socket);

// 处理http请求
bool processHttpRequest(struct HttpRequest* request, struct HttpResponse* response);

// 解码字符串，处理中文字符或者特殊字符
int hexToDec(char c);
void decodeMsg(char* to, char* from);

// 得到对应文件格式的type
const char* getFileType(const char* name);

// 发送文件
void sendFile(const char* fileName, int cfd);
// 发送目录
void sendDir(const char* dirName, int cfd);