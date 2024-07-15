#pragma once
#include <iostream>
#include <string>

using namespace std;

#include "EventLoop.h"
#include "Channel.h"

struct EventLoop;

// 可以将Dispatcher的虚函数定义成纯虚函数，因为后面没有需要创建Dispatcher这个父类的实例的需求
// 如果定义为纯虚函数，那么就不用给这个类中的虚函数结构体了；

class Dispatcher
{
public:
    Dispatcher(EventLoop* evLoop);
    virtual ~Dispatcher();
    // 添加
    virtual int add(); //通过EventLoop* evLoop取到不同的epoll,poll或者select需要的数据块
    // 删除
    virtual int remove();
    // 修改
    virtual int modify();
    // 事件监测
    virtual int dispatch(int timeout); // timeout单位 s

    inline void setChannel(Channel* channel) {
        m_channel = channel;
    }

protected:
    string m_name = string();
    EventLoop* m_evLoop;
    Channel* m_channel;

};

