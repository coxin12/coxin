/******************************************************************
 * Copyright by SunRise, 2013-11-07
 * File Name: Queue.cpp
 *
 * Modified History:
 *     See ChangeLog.
 ******************************************************************/

#ifndef _QUEUE_H
#define _QUEUE_H

#include <pthread.h>
#include <queue>
#include <TransferTask.h>

class Queue : public std::queue<TransferTask> {
public:
    Queue();
    Queue(std::vector<TransferTask> taskArray);
    virtual ~Queue();

    void Push(const TransferTask &task);
    TransferTask Pop();
    TransferTask TimePop();
private:
    pthread_mutex_t m_Mutex;
    pthread_cond_t m_Cond;
};

#endif
