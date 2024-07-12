#pragma once

struct Buffer {
    // 指向内存的指针
    char* data;
    int capacity;
    int readPos;
    int writePos;
};

// 初始化
struct Buffer* bufferInit(int size);
void bufferDestroy(struct Buffer* buffer);

// 扩容
void bufferExtendRoom(struct Buffer* buffer, int size);
// 得到的剩余的可写的内存容量
int bufferWriteableSize(struct Buffer* buffer);
// 得到的剩余的可读的内存容量
int bufferReadableSize(struct Buffer* buffer);

// 写内存 
    // 1.直接写 
int bufferAppendData(struct Buffer* buffer, const char* data, int size);
int bufferAppendString(struct Buffer* buffer, const char* data);
    // 2.接收套接字数据
int bufferSocketRead(struct Buffer* buffer, int fd);




