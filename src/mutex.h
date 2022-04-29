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
    typedef ScopedLockImpl<Mutex> Lock;
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

template<typename T>
class ConditionImpl {
public:
    /**
     * @brief Construct a new Condtion Impl object
     * @param con condtion variant
     */
    ConditionImpl(T& con);

    /**
     * @brief Destroy the Condtion Impl object
     */
    ~ConditionImpl();

    /**
     * @brief signal
     */
    void signal();

    /**
     * @brief broad cast
     */
    void broadcast();

    /**
     * @brief 
     */
    void wait();

private:
    /// condition variant
    T& con_;
    /// condition is blocked
    bool blocked_ {false};
};

class ConditionBlock : Noncopyable {
public:
    /**
     * @brief Construct a new Condition Block object
     */
    ConditionBlock();

    /**
     * @brief Destroy the Condition Block object
     */
    ~ConditionBlock();

    void signal();

    void broadcast();

    void wait();

private:
    /// condition variant
    pthread_cond_t cond_;
};


}


#endif