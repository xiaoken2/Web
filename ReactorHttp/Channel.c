#include "Channel.h"
#include <stdlib.h>

struct Channel* channelInit(int fd, int events, handleFunc readCallback, handleFunc writeCallback, 
handleFunc destroyCallback, void* arg) {
    struct Channel* channel = (struct Channel*)malloc(sizeof(struct Channel));
    channel->fd =fd;
    channel->events = events;
    channel->readCallback = readCallback;
    channel->writeCallback = writeCallback;
    channel->destroyCallback = destroyCallback;
    channel->arg = arg;
    
    return channel;
}

void writeEventEnable(struct Channel* channel, bool flag) {
    if (flag) {
        channel->events |= WriteEvent;
    } else {
        channel->events = channel->events & ~WriteEvent;
    }
}

bool isWriteEventEnable(struct Channel* channel) {
    return channel->events & WriteEvent;
}
