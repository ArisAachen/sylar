#include "mutex.h"

#include <pthread.h>


namespace sylar {

Mutex::Mutex() {
    pthread_mutex_init(&mutex_, nullptr);
}

Mutex::~Mutex() {
    pthread_mutex_destroy(&mutex_);
}

void Mutex::lock() {
    pthread_mutex_lock(&mutex_);
}

void Mutex::unlock() {
    pthread_mutex_unlock(&mutex_);
}

ConditionBlock::ConditionBlock() {
    // init condition
    pthread_cond_init(&cond_, nullptr);
    // init mutex
    pthread_mutex_init(&mutex_, nullptr);
}

ConditionBlock::~ConditionBlock() {
    // destory condition
    pthread_cond_destroy(&cond_);
    // mutex destory
    pthread_mutex_destroy(&mutex_);
}

void ConditionBlock::signal() {
    if (!blocked_) 
        return;
    blocked_ = false;
    // signal once
    pthread_mutex_lock(&mutex_);
    pthread_cond_signal(&cond_);
    pthread_mutex_unlock(&mutex_);
}

void ConditionBlock::broadcast() {
    if (!blocked_) 
        return;
    blocked_ = false;
    // broadcast signal
    pthread_mutex_lock(&mutex_);
    pthread_cond_broadcast(&cond_);
    pthread_mutex_unlock(&mutex_);
}

void ConditionBlock::wait() {
    // already has one block
    if (blocked_) 
        return;
    blocked_ = true;
    pthread_cond_wait(&cond_, &mutex_);
}

}