#define _GNU_SOURCE
#include <stdio.h>
#include <strings.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>
#include <ctype.h>

#include "HttpRequest.h"


char* HttpRequest::splitRequestLine(const char* start, const char* end, const char* sub, function<void(string)> callback) {
    char* space = const_cast<char*> (end);
    if (sub != nullptr) {
        space = static_cast<char*> (memmem(start, end - start, sub, strlen(sub)));
        assert(space != nullptr);
    }
    int length = space - start;
    callback(string(start, length));

    return space + 1;
}

HttpRequest::HttpRequest()
{
    reset();
}

HttpRequest::~HttpRequest()
{
}


void HttpRequest::reset()
{
    m_curState = PrecessState::ParseReqLine;
    m_method = m_url = m_version = string();
    m_reqHeaders.clear();
}


void HttpRequest::addHeader(const string key, const string value)
{
    if (key.empty() || value.empty()) {
        return;
    }
    m_reqHeaders.insert(make_pair(key, value));
}

string HttpRequest::getHeader(const string key)
{
    auto it = m_reqHeaders.find(key);
    if (it != m_reqHeaders.end()) {
        return m_reqHeaders[key];
    }
    return string();
}

bool HttpRequest::parseRequestLine(Buffer *readBuf)
{
    // 读出请求数据
    // 保存字符串起始地址
    char* end = readBuf->findCRLF();
    // 保存字符串起始地址
    char* start = readBuf->data();
    // 请求行总长度
    int LineSize = end - start;

    
    if (LineSize) {
        auto methodFunc = bind(&HttpRequest::setMethod, this, placeholders::_1);
        start = splitRequestLine(start, end, " ", methodFunc);
        auto urlFunc = bind(&HttpRequest::setUrl, this, placeholders::_1);
        start = splitRequestLine(start, end, " ", urlFunc);
        auto versionFunc = bind(&HttpRequest::setVersion, this, placeholders::_1);
        splitRequestLine(start, end, NULL, versionFunc);

        // 为请求头做准备
        readBuf->readPosIncrease(LineSize + 2); 

        // 修改状态
        setState(PrecessState::ParseReqHeaders);
        return true;
    }
    return false;
}

bool HttpRequest::parseRequestHeader(Buffer *readBuf)
{
    char* end = readBuf->findCRLF();;
    if (end != nullptr) {
        char* start = readBuf->data();
        int lineSize = end - start;
        // 基于：搜索字符串
        char* middle = static_cast<char*> (memmem(start, lineSize, ": ", 2));
        if (middle != nullptr) {
            // key值拷贝
            int keyLen = middle - start;
            int valueLen = end - middle - 2;
            if (keyLen > 0 && valueLen > 0) {
                string key(start, keyLen);
                string value(middle + 2, valueLen);
                addHeader(key, value);
            }
            // 移动读数据的位置
            readBuf->readPosIncrease(lineSize + 2);
        }
        else {
            // 请求头已经被解析完了
            readBuf->readPosIncrease(2);
            // 修改解析状态,看是get请求还是post请求
            // 忽略post请求，按照get请求处理
            setState(PrecessState::ParseReqDone);
        }
        return true;
    }
    return false;
}

bool HttpRequest::parseHttpRequest(Buffer *readBuf, HttpResponse *response, Buffer *sendBuf, int socket)
{
    bool flag = true;
    while (m_curState != PrecessState::ParseReqDone) {
        switch (m_curState)
        {
        case PrecessState::ParseReqLine:
            flag =  parseRequestLine(readBuf);
            break;
        case PrecessState::ParseReqHeaders:
            flag = parseRequestHeader(readBuf);
            break;
        case PrecessState::ParseReqBody:
            break;
        default:
            break;
        }

        if (!flag) {
            return flag;
        }
        // 判断是否解析完了，如果解析完了，需要准备回复的数据
        if (m_curState == PrecessState::ParseReqDone) {
            // 1.根据解析的原始数据，对客户端的请求做出处理
            processHttpRequest(response);
            // 2. 组织响应数据并发送给客户端
            response->prepareMsg(sendBuf, socket);
        }
    }
    m_curState = PrecessState::ParseReqLine;  // 状态还原，保证能继续处理第二天及以后的请求
    return flag;
}

bool HttpRequest::processHttpRequest(HttpResponse *response)
{
    // 解析请求行 get /xxx/1.jpg http/1.1
    if (strcasecmp(m_method.data(), "get") != 0) {
        return false;
    }

    m_url = decodeMsg(m_url);
    // 处理客户端请求的静态资源（目录或者文件）
    const char* file = NULL;
    if (strcmp(m_url.data(), "/") == 0) {
        file = "./";
    } else {
        file = m_url.data() + 1;
    }

    // 获取文件属性
    struct stat st;
    int ret = stat(file, &st);
    if (ret == -1) {
        // 文件不存在 --回复404
        // sendHeadmsg(cfd, 404, "Not Found", getFileType(".html"), -1);
        // sendFile("404.html", cfd);
        // 状态行
        response->setFileName("404.html");
        response->setStateCode(StatusCode::NotFound);
        // 响应头
        response->addHeader("Content-type", getFileType(".html"));
        response->sendDataFunc = sendFile;
        return 0;
    }
    // 判断文件类型
    response->setFileName("file");
    response->setStateCode(StatusCode::OK);
    if (S_ISDIR(st.st_mode)) {  // 如果是目录S_ISDIR()返回1，否则返回0
        // 把这个目录的内容发送给客户端
        // sendHeadmsg(cfd, 200, "OK", getFileType(".html"), -1);
        // sendDir(file, cfd);
        // 响应头
        response->addHeader("Content-type", getFileType(".html"));
        response->sendDataFunc = sendDir;

    } else {
        // 把这个文件发送给客户端
        // sendHeadmsg(cfd, 200, "Ok", getFileType(file), st.st_size);
        // sendFile(file, cfd);
        // 响应头
        response->addHeader("Content-type", getFileType(file));
        response->addHeader("Content-length", to_string(st.st_size));
        response->sendDataFunc = sendFile;
    }
    return false;
}

string HttpRequest::decodeMsg(string msg)
{
    string str = string();
    const char* from = msg.data();
    for (; *from != '\0'; ++from)
    {
        // isxdigit -> 判断字符是不是16进制格式, 取值在 0-f
        // Linux%E5%86%85%E6%A0%B8.jpg
        if (from[0] == '%' && isxdigit(from[1]) && isxdigit(from[2]))
        {
            // 将16进制的数 -> 十进制 将这个数值赋值给了字符 int -> char
            // B2 == 178
            // 将3个字符, 变成了一个字符, 这个字符就是原始数据
            str.append(1, hexToDec(from[1]) * 16 + hexToDec(from[2]));

            // 跳过 from[1] 和 from[2] 因此在当前循环中已经处理过了
            from += 2;
        }
        else
        {
            // 字符拷贝, 赋值
            str.append(1, *from);
        }

    }
    // str += '\0';
    str.append(1, '\0');
    return str;
}

const string HttpRequest::getFileType(const string name)
{
    const char* dot = strrchr(name.data(), '.');
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
    return string();
}

void HttpRequest::sendFile(string fileName, Buffer *sendBuf, int cfd)
{
    int fd = open(fileName.data(), O_RDONLY);
    assert(fd > 0);
    while (1) {
        char buf[1024];
        int len = read(fd, buf, sizeof buf);
        if (len > 0) {
            // send(cfd, buf, len, 0);
            sendBuf->appendString(buf, len);
#ifndef MSG_SEND_AUTO
            sendBuf->sendData(cfd);
#endif
            // usleep(10); // 防止发送端发送数据太快，接收端处理不过来造成堵塞
        } else if (len == 0) {
            break; 
        } else {
            close(cfd);
            perror("read");
        }

    }

    // off_t offset = 0;
    // int size = lseek(fd, 0, SEEK_END);
    // lseek(fd, 0, SEEK_SET);
    // while (offset < size) {
    //     int ret = sendfile(cfd, fd, &offset, size);
    //     printf("%d\n", ret);
    //     if (ret == -1) {
    //         perror("sendfile");
    //     }
    // }
    close(fd);
}

void HttpRequest::sendDir(string dirName, Buffer *sendBuf, int cfd)
{
    char buf[4096] = {0};
    sprintf(buf, "<html><head><title>%s</title></head><body><table>", dirName.data());
    struct dirent** namelist;
    int num = scandir(dirName.data(), &namelist, NULL, alphasort);
    for (int i = 0; i < num; ++i) {
        // 取出的namelist指向的是一个指针数组 struct dirent* tmp[]
        char* name = namelist[i]->d_name;
        struct stat st;
        char subPath[1024] = {0};
        sprintf(subPath, "%s/%s", dirName, name);
        stat(subPath, &st);

        if (S_ISDIR(st.st_mode)) {
            // a标签<a href="地址"name</a>> 可以实现跳转到指定子目录

            sprintf(buf + strlen(buf), 
            "<tr><td><a href=\"%s/\">%s</td><td>%ld</td></tr>", 
            name, name, st.st_size);
        } else {
            // 非目录
            sprintf(buf + strlen(buf), 
            "<tr><td><a href=\"%s\">%s</td><td>%ld</td></tr>", 
            name, name, st.st_size);
        }
        // send(cfd, buf, strlen(buf), 0);
        sendBuf->appendString(buf);
#ifndef MSG_SEND_AUTO
        sendBuf->sendData(cfd);
#endif
        memset(buf, 0, sizeof(buf));
        free(namelist[i]);
    }
    sprintf(buf, "</table></body></html>");
    // send(cfd, buf, strlen(buf), 0);
    sendBuf->appendString(buf);
#ifndef MSG_SEND_AUTO
    sendBuf->sendData(cfd);
#endif
    free(namelist);

}

int HttpRequest::hexToDec(char c)
{
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return 0;
}
