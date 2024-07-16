#pragma once
#include <thread>
#include <queue>
#include <map>
#include <mutex>

using namespace std;

#include "Dispatcher.h"
#include "Channel.h"

// 处理节点中的channel
enum class ElemType:char{ADD, DELETE, MODIFY};

// 定义任务节点
struct ChannelElement
{
    ElemType type;  // 如何处理节点中的channel
    struct Channel* channel;
};

class Dispatcher;
class EventLoop{
public:
    EventLoop();
    EventLoop(const string threadName);
    ~EventLoop();

    // 启动反应堆模型
    int run();

    // 处理激活的文件fd
    int eventActivate(int fd, int event);

    // 添加任务到任务队列
    int addTask(Channel* channel, ElemType type);

    // 处理任务队列中的任务
    int processTaskQ();

    // 处理dispatcher中的节点
    int add(Channel* channel);
    int remove(Channel* channel);
    int modify(Channel* channel);
    // 释放channel
    int freeChannel(Channel* channel);

    static int readLocalMessage(void* arg);
    int readMessage();

    // 返回线程ID
    inline thread::id getThreadID() {
        return m_threadID;
    }

private:
    void taskWakeup();

private:
    bool m_isQuit;
    // 该指针指向子类的实例：epool，pool，select
    Dispatcher* m_dispatcher;
    // 任务队列
    queue<ChannelElement*> m_taskQ;
    // map来映射文件描述符和channel之间的关系
    map<int, Channel*> m_channelMap;
    // 线程id，name
    thread::id m_threadID;
    string m_threadName;
    mutex m_mutex;

    // 用于唤醒阻塞中的子线程
    // 用于存储初始化的两个文件描述符
    int m_socketPair[2];
    
};
