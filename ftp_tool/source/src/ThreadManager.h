/*
 * ThreadManager.h
 *
 *  Created on: 2013Äê11ÔÂ25ÈÕ
 *      Author: liangjianqiang
 */

#ifndef THREADMANAGER_H_
#define THREADMANAGER_H_

#include <time.h>
#include <pthread.h>
#include <stdexcept>
#include <vector>
#include <map>
#include "Mutex.h"

class ThreadManager {
public:
    typedef void* (*thread_routine_t)(void*);
    struct ThreadInfo {
        time_t lastRunTime;
        bool isActive;
        time_t totalRunTime;
    };
    typedef std::map<pthread_t, ThreadInfo>::const_iterator ThreadIterator;
protected:
    ThreadManager();
public:
    virtual ~ThreadManager();

    static ThreadManager *getInstance();

    pthread_t createThread(thread_routine_t threadPtr, void *threadParam);
    void releaseThread(pthread_t tid);

    size_t getThreadCount();
    void setLastRunTime(pthread_t tid, time_t last_run_time);
    void setIsActive(pthread_t tid, bool isActive);
    void setTotalRunTime(pthread_t tid, time_t total_run_time);
    time_t getLastRunTime(pthread_t tid);
    bool isThreadActive(pthread_t tid);
    time_t getTotalRunTime(pthread_t tid);

    ThreadIterator begin() {
        return threadInfos.begin();
    }

    ThreadIterator end() {
        return threadInfos.end();
    }
private:
    static ThreadManager *manager;

    Mutex mutex;
    std::map<pthread_t, ThreadInfo> threadInfos;
};

#endif /* THREADMANAGER_H_ */
