#ifndef __SYLAR_SRC_MUTEX_H__
#define __SYLAR_SRC_MUTEX_H__

#include "noncopyable.h"
#include <pthread.h>


namespace sylar {


template<typename T>
class ScopedLockImpl {
public:
    /**
     * @brief Construct a new Scoped Lock Impl object
     * @param[in] lock lock
     */
    ScopedLockImpl(T & lock);

    /**
     * @brief Destroy the Scoped Lock Impl object
     */
    ~ScopedLockImpl();

    /**
     * @brief lock
     */
    void lock();

    /**
     * @brief unlock
     */
    void unlock();

private:
    /// lock member
    T& lock_;
    /// lock statae
    bool locked_ {false};
};

class Mutex : Noncopyable {
public:
    /**
     * @brief Construct a new Mutex object
     */
    Mutex();

    /**
     * @brief Destroy the Mutex object
     */
    ~Mutex();

    /**
     * @brief lock 
     */
    void lock();

    /**
     * @brief unlock
     */
    void unlock();

private:
    /// pthread mutex
    pthread_mutex_t mutex_;
};




}


#endif