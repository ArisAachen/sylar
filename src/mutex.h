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
    ScopedLockImpl(T& lck): lock_(lck) {
        lock();
    }

    /**
     * @brief Destroy the Scoped Lock Impl object
     */
    ~ScopedLockImpl() {
        unlock();
    }

    /**
     * @brief lock
     */
    void lock() {
        // check if is locked
        if (locked_) 
            return;
        // lock 
        locked_ = true;
        lock_.lock();
    }

    /**
     * @brief unlock
     */
    void unlock() {
        // check if is unlocked
        if (!locked_)
            return;
        locked_ = false;
        lock_.unlock();        
    }

private:
    /// lock member
    T& lock_;
    /// lock statae
    bool locked_ {false};
};

template<typename T>
class ReadScopeLockImpl {
public:
    ReadScopeLockImpl(T& lck): lock_(lck) {
        lock();
    }

    ~ReadScopeLockImpl() {
        unlock();
    }

    void lock() {
        // check if is locked
        if (locked_) 
            return;
        // lock 
        locked_ = true;
        lock_.rdlock();
    }

    void unlock() {
        // check if is unlocked
        if (!locked_)
            return;
        locked_ = false;
        lock_.unlock();           
    }

private:
    /// lock member
    T& lock_;
    /// lock statae
    bool locked_ {false};
};

template<typename T>
class WriteScopeLockImpl {
public:
    WriteScopeLockImpl(T& lck): lock_(lck) {
        lock();
    }

    ~WriteScopeLockImpl() {
        unlock();
    }

    void lock() {
        // check if is locked
        if (locked_) 
            return;
        // lock 
        locked_ = true;
        lock_.wrlock();
    }

    void unlock() {
        // check if is unlocked
        if (!locked_)
            return;
        locked_ = false;
        lock_.unlock();           
    }

private:
    /// lock member
    T& lock_;
    /// lock statae
    bool locked_ {false};
};


template<typename T>
class ConditionImpl {
public:
    /**
     * @brief Construct a new Condtion Impl object
     * @param con condtion variant
     */
    ConditionImpl(T& con) : con_(con) {

    }

    /**
     * @brief Destroy the Condtion Impl object
     */
    ~ConditionImpl() {}

    /**
     * @brief signal
     */
    void signal() {
        con_.signal();
    }

    /**
     * @brief broad cast
     */
    void broadcast() {
        con_.broadcast();
    }

    /**
     * @brief 
     */
    void wait() {
        con_.wait();
    }

private:
    /// condition variant
    T& con_;
    /// condition is blocked
    bool blocked_ {false};
};

class ConditionBlock;

class RWMutex : Noncopyable {
public:
    typedef ReadScopeLockImpl<RWMutex> ReadLock;
    typedef WriteScopeLockImpl<RWMutex> WriteLock;

    /**
     * @brief Construct a new RWMutex object
     */
    RWMutex();

    /**
     * @brief add read lock
     */
    void rdlock();

    /**
     * @brief add write lock
     */
    void wrlock();

    /**
     * @brief unlock lock
     */
    void unlock();

    /**
     * @brief Destroy the RWMutex object
     */
    ~RWMutex();
private:
    pthread_rwlock_t lock_;
};


class Mutex : Noncopyable {
public:
    friend class ConditionBlock;
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

class ConditionBlock : Noncopyable {
public:
    typedef ConditionImpl<ConditionBlock> Block;
    /**
     * @brief Construct a new Condition Block object
     */
    
    ConditionBlock(Mutex& mutex);

    /**
     * @brief Destroy the Condition Block object
     */
    ~ConditionBlock();

    /**
     * @brief emit signal
     */
    void signal();

    /**
     * @brief broadcast signal
     */
    void broadcast();

    /**
     * @brief wait signal
     */
    void wait();    
private:
    /// condition variant
    pthread_cond_t cond_;
    /// mutex
    pthread_mutex_t mutex_;
    /// block state
    bool blocked_;
};




}


#endif