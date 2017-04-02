/*
 * Mutex.h
 *
 *  Created on: 2014��1��28��
 *      Author: liangjianqiang
 */

#ifndef MUTEX_H_
#define MUTEX_H_

#include <pthread.h>

class Mutex {
public:
    Mutex();
    virtual ~Mutex();
    int lock() const;
    int unlock() const;
protected:
    pthread_mutex_t _mutex;
private:
    Mutex(const Mutex& rhs);
    void operator =(const Mutex& rhs);
};

#endif /* MUTEX_H_ */
