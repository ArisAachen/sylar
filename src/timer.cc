#include "timer.h"
#include "log.h"

namespace sylar {


Timer::Timer(uint64_t inter, bool recurring, std::function<void()> cb, std::string name): 
interval_(inter), recurring_(recurring), cb_(cb), name_(name) {

}

void Timer::cancel() {

}



TimerManager::TimerManager() {
    SYLAR_DEBUG("timer manager is created");
    // clear all timers
    timers_.clear();
}

bool TimerManager::empty() {
    // read lock here
    MutexType::ReadLock lock(mutex_);
    return timers_.empty();
}

void TimerManager::add_timer(uint64_t ms, bool recurring, std::function<void ()> cb, std::string name) {
    // write lock 
    MutexType::WriteLock lock(mutex_);
    // add to set
    timers_.emplace(new Timer(ms, recurring, cb, name));
}



}