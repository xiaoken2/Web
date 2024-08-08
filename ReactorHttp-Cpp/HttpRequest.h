#pragma once
#include <sys/types.h>
#include <map>

#include "HttpResponse.h"
#include "Buffer.h"
#include "TcpConnection.h"


// 请求的键值对
struct RequestHeader{
    char* key;
    char* value;
};

// 当前的解析状态
enum class PrecessState:char{
    ParseReqLine,
    ParseReqHeaders,
    ParseReqBody,
    ParseReqDone
};

// 定义http请求结构体
class HttpRequest
{
public:
    
    // 初始化
    HttpRequest();
    ~HttpRequest();

    // 重置HttpRequest结构体
    void reset();

    // 添加请求头
    void addHeader(const string key, const string value);

    // 根据key得到请求头的value
    string getHeader(const string key);

    // 解析请求行
    bool parseRequestLine(Buffer* readBuf);

    // 解析请求头
    bool parseRequestHeader(Buffer* readBuf);

    // 解析http请求协议
    bool parseHttpRequest(Buffer* readBuf,
    HttpResponse* response, Buffer* sendBuf, int socket);

    // 处理http请求
    bool processHttpRequest(HttpResponse* response);

    // 解码字符串，处理中文字符或者特殊字符
    int hexToDec(char c);
    string decodeMsg(string from);

    // 得到对应文件格式的type
    const string getFileType(const string name);

    // 发送文件
    void sendFile(string fileName, Buffer* sendBuf, int cfd);
    // 发送目录
    void sendDir(string dirName, Buffer* sendBuf, int cfd);

    inline void setMethod(string method) {
        m_method = method;
    }

    inline void setUrl(string url) {
        m_url = url;
    }

    inline void setVersion(string version) {
        m_version = version;
    }

        // 获取处理状态的函数
    inline PrecessState getState() {
        return m_curState;
    }

    // 修改处理状态
    inline void setState(PrecessState state) {
        m_curState = state;
    }

private:
    char* splitRequestLine(const char* start, const char* end, 
        const char* sub, function<void(string)> callback);
private:
    string m_method;
    string m_url;
    string m_version;
    map<string, string> m_reqHeaders;
    PrecessState m_curState;
};
