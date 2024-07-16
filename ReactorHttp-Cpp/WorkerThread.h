#pragma once
#include <pthread.h>
#include <thread>
#include <mutex>
#include <condition_variable>

using namespace std;

#include "EventLoop.h"

// 定义子线程的结构体
class WorkerThread {
public:
    WorkerThread(int index);
    ~WorkerThread();

    // 启动线程
    void run();

    inline EventLoop* getEventLoop() {
        return m_evLoop;
    }
private:
    void running();
private:
    thread* m_thread; // 保存线程实例地址的指针
    thread::id m_threadID;  // 线程Id
    string m_name;       // 线程名字
    mutex m_mutex;  //互斥锁
    condition_variable m_cond;  //条件变量
    EventLoop* m_evLoop;  // 反应堆模型
};



