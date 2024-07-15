#pragma once
#include <iostream>
#include <string>

#include "EventLoop.h"
#include "Channel.h"

using namespace std;

struct EventLoop;

// 可以将Dispatcher的虚函数定义成纯虚函数，因为后面没有需要创建Dispatcher这个父类的实例的需求
// 如果定义为纯虚函数，那么就不用给这个类中的虚函数结构体了；

class EpollDispatcher : public Dispatcher
{
public:
    EpollDispatcher(EventLoop* evLoop);
    ~EpollDispatcher();
    // 添加
    int add() override; 
    // 删除
    int remove() override;
    // 修改
    int modify() override;
    // 事件监测
    int dispatch(int timeout); // timeout单位 s
private:
    int epollCtl(int op);

private:
    int m_epfd;
    struct epoll_event* m_events;
    const int m_maxNode = 520;
};


