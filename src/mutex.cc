#include "mutex.h"
#include <pthread.h>


namespace sylar {



template<typename T>
ScopedLockImpl<T>::ScopedLockImpl(T& lc) :
lock_(lc) {
    lock();
}

template<typename T>
ScopedLockImpl<T>::~ScopedLockImpl() {
    unlock();
}

template<typename T>
void ScopedLockImpl<T>::lock() {
    // check if is locked
    if (locked_) 
        return;
    // lock 
    locked_ = true;
    lock_.lock();
}


template<typename T>
void ScopedLockImpl<T>::unlock() {
    // check if is unlocked
    if (!locked_)
        return;
    locked_ = false;
    lock_.unlock();
}

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

template<typename T>
ConditionImpl<T>::ConditionImpl(T& con):
con_(con) {

}

template<typename T>
ConditionImpl<T>::~ConditionImpl() {
    
}


}