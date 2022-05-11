#include "mutex.h"
#include "log.h"

#include <iostream>
#include <exception>


#include <pthread.h>


namespace sylar {

Mutex::Mutex() {
    pthread_mutex_init(&mutex_, nullptr);
}

Mutex::~Mutex() {
    pthread_mutex_destroy(&mutex_);
}

void Mutex::lock() {
    try {
        pthread_mutex_lock(&mutex_);
    } catch (std::exception& e) {
        std::cout << "lock failed " << e.what() << std::endl;
    }
}

void Mutex::unlock() {
    pthread_mutex_unlock(&mutex_);
}

RWMutex::RWMutex() {
    pthread_rwlock_init(&lock_, nullptr);
}

RWMutex::~RWMutex() {
    pthread_rwlock_destroy(&lock_);
}

void RWMutex::rdlock() {
    pthread_rwlock_rdlock(&lock_);
}

void RWMutex::wrlock() {
    pthread_rwlock_wrlock(&lock_);
}

void RWMutex::unlock() {
    pthread_rwlock_unlock(&lock_);
}

ConditionBlock::ConditionBlock(Mutex& mutex) {
    // init condition
    pthread_cond_init(&cond_, nullptr);
    // init mutex
    mutex_ = mutex.mutex_;
}

ConditionBlock::~ConditionBlock() {
    // destory condition
    pthread_cond_destroy(&cond_);
}

void ConditionBlock::signal() {
    if (!blocked_) 
        return;
    blocked_ = false;
    // signal once
    pthread_cond_signal(&cond_);
}

void ConditionBlock::broadcast() {
    if (!blocked_) 
        return;
    blocked_ = false;
    pthread_cond_broadcast(&cond_);
}

void ConditionBlock::wait() {
    blocked_ = true;
    pthread_mutex_lock(&mutex_);
    pthread_cond_wait(&cond_, &mutex_);
    pthread_mutex_unlock(&mutex_);
}

}