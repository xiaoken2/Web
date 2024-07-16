#pragma once
#include <functional>


// 定义函数指针
// typedef int (*handleFunc) (void* arg);
// using handleFunc = int(*)(void*); // C++11新特性

// 定义文件描述符的读写事件
enum class FDEvent {
    TimeOut = 0x01,
    ReadEvent = 0x02,
    WriteEvent = 0x04
};

// 可调用对象包装器打包的是什么？1. 函数指针 2. 可调用对象（可以像函数一样使用）
// 最终得到了地址,但是没有调用
class Channel
{
public:
    using handleFunc = std::function<int (void*)>;
    // 构造函数就是初始化
    Channel (int fd, FDEvent events, handleFunc readCallback, handleFunc writeCallback, handleFunc destroyCallback, void* arg);
    // 回调函数
    handleFunc readCallback;
    handleFunc writeCallback;
    handleFunc destroyCallback;
    // 初始化Channel实例

    // 修改fd的写事件（检测或者不检测）
    void writeEventEnable(bool flag);

    // 判断是否需要检测文件描述符的写事件
    bool isWriteEventEnable();

    // 取出私有成员，内联函数
    inline int getEvent() {
        return m_events;
    }
    inline int getSocket() {
        return m_fd;
    }

    inline const void* getArg() {
        return m_arg;
    }

private:
    // 文件描述符
    int m_fd;
    // 事件(读事件或者写事件)
    int m_events; 
    // 回调函数的参数
    void* m_arg;
};
