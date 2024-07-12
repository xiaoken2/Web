#include <stdio.h>
#include "WorkerThread.h"

int workerThreadInit (struct WorkerThread* thread, int index) {
    thread->evLoop = NULL;
    thread->threadID = 0;
    sprintf(thread->name, "Subthread-%d", index);
    pthread_mutex_init(&thread->mutex, NULL);
    pthread_cond_init(&thread->cond, NULL);
    return 0;
}


// 子线程的回调函数
void * subthreadRunning(void* arg) {
    struct WorkerThread* thread = (struct WorkerThread*)arg;
    pthread_mutex_lock(&thread->mutex);
    thread->evLoop = eventLoopInitEx(thread->name);
    pthread_mutex_unlock(&thread->mutex);
    pthread_cond_signal(&thread->cond);
    eventLoopRun(thread->evLoop);
    return NULL;
}

void workerThreadRun(struct WorkerThread* thread) {
    // 创建子线程
    pthread_create(&thread->threadID, NULL, subthreadRunning, thread);
    // 阻塞主线程，让当前函数不会直接结束，因为要保证子线程的事件循环能够初始化成功
    //这里的共享资源是evLoop
    pthread_mutex_lock(&thread->mutex);
    while (thread->evLoop == NULL) {
        pthread_cond_wait(&thread->cond, &thread->mutex);  
    }
    pthread_mutex_unlock(&thread->mutex);
}


