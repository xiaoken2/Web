#pragma once
#include "Buffer.h"
#include <string>
#include <map>
#include <functional>

using namespace std;

// 定义状态码
enum class StatusCode {
    Unknown,
    OK = 200, 
    MovedPermanently = 301,
    MovedTemporal = 302,
    BadRequest = 400,
    NotFound = 404
};



// 定义结构体
class HttpResponse
{
public:
    // 初始化
    HttpResponse();
    // 销毁函数
    ~HttpResponse();
    // 添加响应头
    void addHeader(const string key, const string value);
    // 组织http响应函数
    void prepareMsg(Buffer* sendBuf, int socket);
    function<void(const string, Buffer*, int)> sendDataFunc;

    inline void setFileName(string name) {
        m_fileName = name;
    }

    inline void setStateCode(StatusCode code) {
        m_statusCode = code;
    }

private:
    // 状态行：状态码，状态描述
    StatusCode m_statusCode;
    string m_fileName;
    // 响应头-键值对
    map<string, string> m_headers;
    // 定义状态码和其对应的描述
    const map<int, string> m_info = {
        {200, "OK"},
        {301, "MovedPermanently"},
        {302, "MovedTemporal"},
        {400, "BadRequest"},
        {404, "NotFound"},
    };
};
