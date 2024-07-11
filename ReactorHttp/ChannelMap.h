#pragma once
#include <stdbool.h>


struct ChannelMap
{
    int size;  // 记录指针指向的数组的元素个数
    // 存储的是struct Channel* list[]
    struct Channel** list;
};

// 初始化
struct ChannelMap* channelMapInit(int size);
// 清空map
void ChannelMapClear(struct ChannelMap* map);
// 因为是静态数组，所以需要扩容
bool makeMapRoom(struct ChannelMap* map, int newSize, int unitSize);
