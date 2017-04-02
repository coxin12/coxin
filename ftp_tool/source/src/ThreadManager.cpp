/*
 * ThreadManager.cpp
 *
 *  Created on: 2013Äê11ÔÂ25ÈÕ
 *      Author: liangjianqiang
 */

#include <string.h>
#include <errno.h>
#include <iostream>
#include "ThreadManager.h"

ThreadManager *ThreadManager::manager = 0;

ThreadManager::ThreadManager() {
}

ThreadManager::~ThreadManager() {
}

ThreadManager *ThreadManager::getInstance() {
    if (manager == 0) {
        manager = new ThreadManager;
    }
    return manager;
}

pthread_t ThreadManager::createThread(thread_routine_t threadPtr,
        void *threadParam) {
    int ret = -1;
    pthread_attr_t attr;
    mutex.lock();

    if ((ret = pthread_attr_init(&attr)) != 0) {
        std::cerr << "pthread_attr_init" << std::endl;
        throw std::runtime_error(strerror(ret));
    }

    if ((ret = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED))
            != 0) {
        std::cerr << "pthread_attr_setdetachstate" << std::endl;
        throw std::runtime_error(strerror(ret));
    }

    pthread_t tid = -1;
    if ((ret = pthread_create(&tid, &attr, threadPtr, threadParam)) != 0) {
        std::cerr << "pthread_create" << std::endl;
        throw std::runtime_error(strerror(ret));
    }

    if ((ret = pthread_attr_destroy(&attr)) != 0) {
        std::cerr << "pthread_attr_destroy" << std::endl;
        throw std::runtime_error(strerror(ret));
    }

    ThreadInfo info;
    info.lastRunTime = 0;
    info.isActive = false;
    info.totalRunTime = 0;
    threadInfos.insert(std::pair<pthread_t, ThreadInfo>(tid, info));

    mutex.unlock();

    return tid;
}

void ThreadManager::releaseThread(pthread_t tid) {
    mutex.lock();

    threadInfos.erase(tid);

    mutex.unlock();
}

size_t ThreadManager::getThreadCount() {
    mutex.lock();
    size_t count = threadInfos.size();
    mutex.unlock();
    return count;
}

void ThreadManager::setLastRunTime(pthread_t tid, time_t last_run_time) {
    mutex.lock();
    if (threadInfos.find(tid) == threadInfos.end()) {
        throw std::out_of_range("setLastRunTime out of range");
    }
    threadInfos[tid].lastRunTime = last_run_time;
    mutex.unlock();
}
void ThreadManager::setIsActive(pthread_t tid, bool isActive) {
    mutex.lock();
    if (threadInfos.find(tid) == threadInfos.end()) {
        throw std::out_of_range("setLastRunTime out of range");
    }
    threadInfos[tid].isActive = isActive;
    mutex.unlock();
}
void ThreadManager::setTotalRunTime(pthread_t tid, time_t total_run_time) {
    mutex.lock();
    if (threadInfos.find(tid) == threadInfos.end()) {
        throw std::out_of_range("setLastRunTime out of range");
    }
    threadInfos[tid].totalRunTime = total_run_time;
    mutex.unlock();
}
time_t ThreadManager::getLastRunTime(pthread_t tid) {
    mutex.lock();
    if (threadInfos.find(tid) == threadInfos.end()) {
        throw std::out_of_range("setLastRunTime out of range");
    }
    mutex.unlock();
    return threadInfos[tid].lastRunTime;
}
bool ThreadManager::isThreadActive(pthread_t tid) {
    mutex.lock();
    if (threadInfos.find(tid) == threadInfos.end()) {
        throw std::out_of_range("setLastRunTime out of range");
    }
    mutex.unlock();
    return threadInfos[tid].isActive;
}
time_t ThreadManager::getTotalRunTime(pthread_t tid) {
    mutex.lock();
    if (threadInfos.find(tid) == threadInfos.end()) {
        throw std::out_of_range("setLastRunTime out of range");
    }
    mutex.unlock();
    return threadInfos[tid].totalRunTime;
}
