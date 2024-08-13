#include "Channel.h"
#include <stdlib.h>


Channel::Channel(int fd, FDEvent events, handleFunc readFunc, handleFunc writeFunc, handleFunc destroyFunc, void *arg)
{
    m_fd =fd;
    m_events = (int)events;
    m_arg = arg;
    readCallback = readFunc;
    writeCallback = writeFunc;
    destroyCallback = destroyFunc;

}

void Channel::writeEventEnable(bool flag)
{
    if (flag) {
        m_events |= (int)FDEvent::WriteEvent; // C语言风格
        // m_events |= static_cast<int> (FDEvent::WriteEvent); // C++风格
    } else {
        m_events = m_events & ~(int)FDEvent::WriteEvent;
    }
}

bool Channel::isWriteEventEnable()
{
    return m_events & (int)FDEvent::WriteEvent;
}
