/*
 * Mutex.cpp
 *
 *  Created on: 2014Äê1ÔÂ28ÈÕ
 *      Author: liangjianqiang
 */

#include "Mutex.h"
#include <string.h>
#include <errno.h>
#include <stdexcept>
#include <string>

Mutex::Mutex() {
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    if (pthread_mutex_init(&_mutex, &attr)) {
        throw std::runtime_error(std::string("pthread_mutex_init: ")
                                    + strerror(errno));
    }
}

Mutex::~Mutex() {
    pthread_mutex_destroy(&_mutex);
}

int Mutex::lock() const {
    return pthread_mutex_lock(const_cast<pthread_mutex_t*>(&_mutex));
}

int Mutex::unlock() const {
    return pthread_mutex_unlock(const_cast<pthread_mutex_t*>(&_mutex));
}
