#include <stdio.h>
#include <stdlib.h>
#include <sys/uio.h>
#include "Buffer.h"

struct Buffer* bufferInit(int size) {
    struct Buffer* buffer = (struct Buffer*)malloc(sizeof(struct Buffer));

    if (buffer != NULL) {
        buffer->data = (char*)malloc(size);
        buffer->capacity = size;
        buffer->readPos = buffer->writePos = 0;
        memset(buffer->data, 0, size);
    }
}

void bufferDestroy(struct Buffer* buffer) {
    if (buffer != NULL) {
        if (buffer->data != NULL) {
            free(buffer->data);
        }
    }
    free(buffer);
}

int bufferWriteableSize(struct Buffer* buffer) {
    return buffer->capacity -  buffer->writePos;
}

int bufferReadableSize(struct Buffer* buffer) {
    return buffer->writePos - buffer->readPos;
}

void bufferExtendRoom(struct Buffer* buffer, int size) {
    // 1. 内存够用，不需要扩容
    if (bufferWriteableSize(buffer) >= size) {
        return;
    } 
    // 2. 内存需要合并才够用->不需要扩容
    else if (bufferWriteableSize(buffer) + buffer->readPos >= size) {
        // 得到未读内存块大小
        int readable = bufferReadableSize(buffer);
        // 移动内存
        memcpy(buffer->data, buffer->data + buffer->readPos, readable);
        buffer->readPos = 0;
        buffer->writePos = readable;
    }
    // 3.内存不够用,需要扩容
    else {
        void * temp = realloc(buffer->data, buffer->capacity + size);
        if (temp == NULL) {
            return;
        }
        memset(temp + buffer->capacity, 0 ,size);
        buffer->data = temp;
        buffer->capacity += size;
    }
}

int bufferAppendData(struct Buffer* buffer, const char* data, int size) {
    if (buffer == NULL || data == NULL || size <= 0) 
    {
        return -1;
    }
    // 扩容(可能不会扩容)
    bufferExtendRoom(buffer, size);
    // 将data的数据拷贝到buffer中
    memcpy(buffer->data + buffer->writePos, data, size);
    buffer->writePos += size;
    return 0;
}

int bufferAppendString(struct Buffer* buffer, const char* data) {
    int size = strlen(data);
    // 扩容
    int ret = bufferAppendData(buffer, data, size);
    return ret;
}

int bufferSocketRead(struct Buffer* buffer, int fd) {
    // read / recv / readv
    struct iovec vec[2];
    // 初始化数组元素
    int writeable = bufferWriteableSize(buffer);
    vec[0].iov_base = buffer->data + buffer->writePos;
    vec[0].iov_len = writeable;
    char* tmpbuf = (char*)malloc(40960);
    vec[1].iov_base = buffer->data + buffer->writePos;
    vec[1].iov_len = 40960;
    int result = readv(fd, vec, 2);
    if (result == -1) {
        return -1;
    } else if (result <= writeable) {
        buffer->writePos += result;
    } else {
        buffer->writePos = buffer->capacity;
        bufferAppendData(buffer, tmpbuf, result - writeable);
    }
    free(tmpbuf);

    return result;
}
