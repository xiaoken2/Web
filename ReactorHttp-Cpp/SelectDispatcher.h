#pragma once
#include <iostream>
#include <string>
#include <sys/select.h>

#include "EventLoop.h"
#include "Channel.h"

using namespace std;

struct EventLoop;

// 可以将Dispatcher的虚函数定义成纯虚函数，因为后面没有需要创建Dispatcher这个父类的实例的需求
// 如果定义为纯虚函数，那么就不用给这个类中的虚函数结构体了；

class SelectDispatcher : public Dispatcher
{
public:
    SelectDispatcher(EventLoop* evLoop);
    ~SelectDispatcher();
    // 添加
    int add() override; 
    // 删除
    int remove() override;
    // 修改
    int modify() override;
    // 事件监测
    int dispatch(int timeout); // timeout单位 s
private:
    void setFdSet();
    int clearFdSet ();
private:
    fd_set m_readSet;
    fd_set m_writSet;
    const int m_maxSize = 1024;
};


