#include "Channel.h"
#include <stdlib.h>


Channel::Channel(int fd, int events, handleFunc readCallback, handleFunc writeCallback, handleFunc destroyCallback, void *arg)
{
    m_fd =fd;
    m_events = events;
    m_arg = arg;
    readCallback = readCallback;
    writeCallback = writeCallback;
    destroyCallback = destroyCallback;

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
