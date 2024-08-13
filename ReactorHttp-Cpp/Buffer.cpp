// #define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <sys/uio.h>
#include <strings.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>

#include "Buffer.h"


Buffer::Buffer(int size) :m_capacity(size)
{
    m_data = (char*)malloc(size);  // 为什么不用new，因为后续要用到realloc进行扩容，它和malloc是对应的一套函数
    bzero(m_data, size);
}

Buffer::~Buffer()
{
    if (m_data != NULL) {
        free(m_data);
    }
}

void Buffer::extendRoom(int size)
{
    // 1. 内存够用，不需要扩容
    if (writeableSize() >= size) {
        return;
    } 
    // 2. 内存需要合并才够用->不需要扩容
    else if (writeableSize() + m_readPos >= size) {
        // 得到未读内存块大小
        int readable = readableSize();
        // 移动内存
        memcpy(m_data, m_data + m_readPos, readable);
        m_readPos = 0;
        m_writePos = readable;
    }
    // 3.内存不够用,需要扩容
    else {
        void * temp = realloc(m_data, m_capacity + size);
        if (temp == NULL) {
            return;
        }
        memset((char*)temp + m_capacity, 0 ,size);
        // m_data = (char*)temp;
        m_data = static_cast<char*>(temp);
        m_capacity += size;
    }
}


int Buffer::appendString(const char *data, int size)
{
    if (data == NULL || size <= 0) 
    {
        return -1;
    }
    // 扩容(可能不会扩容)
    extendRoom(size);
    // 将data的数据拷贝到buffer中
    memcpy(m_data + m_writePos, data, size);
    m_writePos += size;
    return 0;
}

int Buffer::appendString(const char *data)
{
    int size = strlen(data);
    // 扩容
    int ret = appendString(data, size);
    return ret;
}

int Buffer::appendString(const string data)
{
    int ret = appendString(data.data());
    return ret;
}

int Buffer::socketRead(int fd)
{
    // read / recv / readv
    struct iovec vec[2];
    // 初始化数组元素
    int writeable = writeableSize();
    vec[0].iov_base = m_data + m_writePos;
    vec[0].iov_len = writeable;
    char* tmpbuf = (char*)malloc(40960);
    vec[1].iov_base = m_data + m_writePos;
    vec[1].iov_len = 40960;
    int result = readv(fd, vec, 2);
    if (result == -1) {
        return -1;
    } else if (result <= writeable) {
        m_writePos += result;
    } else {
        m_writePos = m_capacity;
        appendString(tmpbuf, result - writeable);
    }
    free(tmpbuf);

    return result;
}

char *Buffer::findCRLF()
{
    // strstr --> 大字符串中匹配子字符串(遇到\0结束) char *strstr(const char *haystack, const char *needle);
    // memmem --> 大数据块中匹配子数据块(需要指定数据块大小)
    // void *memmem(const void *haystack, size_t haystacklen,
    //      const void* needle, size_t needlelen);

    char* ptr = static_cast<char*>(memmem(m_data+m_readPos, readableSize(), "\r\n", 2));
    return ptr;
}

int Buffer::sendData(int socket)
{
    // 判断有无可读数据
    int readable = readableSize();
    if (readable > 0) {
        int count = send(socket, m_data + m_readPos, readable, MSG_NOSIGNAL);
        if (count) {
            m_readPos += count;
            usleep(1);
        }
        return count;
    }
    return 0;
}

