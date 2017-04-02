/******************************************************************
 * Copyright by SunRise, 2013-11-07
 * File Name: Queue.cpp
 *
 * Modified History:
 *     See ChangeLog.
 ******************************************************************/

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "Queue.h"

Queue::Queue() {
    int retval = -1;
    if ((retval = pthread_mutex_init(&m_Mutex, 0)) != 0) {
        fprintf(stderr, "Queue[Queue] pthread_mutex_init: %s",
                strerror(retval));
        exit(-1);
    }

    if ((retval = pthread_cond_init(&m_Cond, 0)) != 0) {
        fprintf(stderr, "Queue[Queue] pthread_cond_init: %s", strerror(retval));
        exit(-1);
    }
}

Queue::Queue(std::vector<TransferTask> taskArray) {
    for (size_t i = 0; i < taskArray.size(); i++) {
        this->push(taskArray[i]);
    }
    int retval = -1;
    if ((retval = pthread_mutex_init(&m_Mutex, 0)) != 0) {
        fprintf(stderr, "Queue[Queue] pthread_mutex_init: %s",
                strerror(retval));
        exit(-1);
    }

    if ((retval = pthread_cond_init(&m_Cond, 0)) != 0) {
        fprintf(stderr, "Queue[Queue] pthread_cond_init: %s", strerror(retval));
        exit(-1);
    }
}

Queue::~Queue() {
    int retval = -1;
    if ((retval = pthread_mutex_destroy(&m_Mutex)) != 0) {
        fprintf(stderr, "Queue[~Queue] pthread_mutex_destroy: %s",
                strerror(retval));
        exit(-1);
    }

    if ((retval = pthread_cond_destroy(&m_Cond)) != 0) {
        fprintf(stderr, "Queue[~Queue] pthread_cond_destroy: %s",
                strerror(retval));
        exit(-1);
    }
}

void Queue::Push(const TransferTask &task) {
    int retval;

    if ((retval = pthread_mutex_lock(&m_Mutex)) != 0) {
        fprintf(stderr, "Queue[Push] pthread_mutex_lock: %s", strerror(retval));
    }

    push(task);

    pthread_cond_broadcast(&m_Cond);

    if ((retval = pthread_mutex_unlock(&m_Mutex)) != 0) {
        fprintf(stderr, "Queue[Push] pthread_mutex_unlock: %s",
                strerror(retval));
    }
}

TransferTask Queue::Pop() {
    int retval;
    if ((retval = pthread_mutex_lock(&m_Mutex)) != 0) {
        fprintf(stderr, "Queue[Pop] pthread_mutex_lock: %s", strerror(retval));
    }

    while (this->empty()) {
        if ((retval = pthread_cond_wait(&m_Cond, &m_Mutex)) != 0) {
            fprintf(stderr, "Queue[Pop] pthread_cond_wait: %s",
                    strerror(retval));
        }
    }

    TransferTask task = front();
    pop();

    if ((retval = pthread_mutex_unlock(&m_Mutex)) != 0) {
        fprintf(stderr, "Queue[Pop] pthread_mutex_unlock: %s",
                strerror(retval));
    }

    return task;
}

TransferTask Queue::TimePop() {
    int retval;
    if ((retval = pthread_mutex_lock(&m_Mutex)) != 0) {
        throw std::runtime_error(strerror(retval));
    }

    if (this->empty()) {
        struct timespec abstime;
        abstime.tv_sec = time(0) + 10;
        abstime.tv_nsec = 0;

        retval = pthread_cond_timedwait(&m_Cond, &m_Mutex, &abstime);
        if (ETIMEDOUT == retval) {
            if ((retval = pthread_mutex_unlock(&m_Mutex)) != 0) {
                throw std::runtime_error(strerror(retval));
            }
            throw std::runtime_error("Queue is empty and exit by timeout");
        }
        if ((0 != retval) && (ETIMEDOUT != retval)) {
            throw std::runtime_error(strerror(retval));
        }
        if (this->empty()) {
            throw std::runtime_error("Queue is empty");
        }
    }

    TransferTask task = front();
    pop();

    if ((retval = pthread_mutex_unlock(&m_Mutex)) != 0) {
        throw std::runtime_error(strerror(retval));
    }

    return task;
}

