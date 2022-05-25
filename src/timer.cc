#include "timer.h"
#include "log.h"
#include "utils.h"

#include <cstdio>
#include <iterator>
#include <memory>
#include <vector>
#include <algorithm>
#include <cstdint>
#include <functional>


namespace sylar {


Timer::Timer(uint64_t inter, bool recurring, std::function<void()> cb, TimerManager* mgr, std::string name): 
    recurring_(recurring), cb_(cb), timer_mgr_(mgr), name_(name) {
    ms_ = SystemInfo::get_elapsed() + inter;
    SYLAR_FMT_DEBUG("create timer, name: %s, time: %ld", name_.c_str(), inter);
}

void Timer::cancel() {
    SYLAR_FMT_DEBUG("cancel timer, name: %s", name_.c_str());
    if (timer_mgr_)
        timer_mgr_->del_timer(shared_from_this());
}

void Timer::reset(uint64_t inter) {
    ms_ = SystemInfo::get_elapsed() + inter;
    SYLAR_FMT_DEBUG("reset timer, name: %s, time: %ld", name_.c_str(), inter);
    if (timer_mgr_) {
        // TODO: could optimize here
        timer_mgr_->del_timer(shared_from_this());
        timer_mgr_->add_timer(shared_from_this());
    }
}

TimerManager::TimerManager() {
    SYLAR_DEBUG("timer manager is created");
    // clear all timers
    timers_.clear();
}

TimerManager::~TimerManager() {
    timers_.clear();
}

bool TimerManager::empty() {
    // read lock here
    MutexType::ReadLock lock(mutex_);
    return timers_.empty();
}

void TimerManager::add_timer(uint64_t ms, bool recurring, std::function<void()> cb, std::string name) {
    // write lock 
    MutexType::WriteLock lock(mutex_);
    // add to set
    timers_.emplace(new Timer(ms, recurring, cb, this, name));
}

void TimerManager::add_timer(Timer::ptr timer) {
    // write lock 
    MutexType::WriteLock lock(mutex_);
    timers_.insert(timer);
}

void TimerManager::del_timer(Timer::ptr timer) {
    // write lock
    MutexType::WriteLock lock(mutex_);
    timers_.erase(timer);
}

// condition execute
static void OnTimer(std::shared_ptr<void> cond, std::function<void()> cb) {
    if (cond) 
        cb();
}

// list all expired cb
void TimerManager::list_expired_cb(std::vector<std::function<void()>> cbs) {
    // get current time
    uint64_t ms_now = SystemInfo::get_elapsed();
    if (empty()) {
        cbs.clear();
        return;
    }
    // lock 
    std::vector<Timer::ptr> expired {};
    MutexType::ReadLock lock(mutex_);
    Timer::ptr now_timer(new Timer(ms_now));
    // search for the last place
    auto pos = timers_.upper_bound(now_timer);
    std::copy(timers_.begin(), timers_.end(), std::back_inserter(expired));
    cbs.reserve(expired.size());
    // add to end
    std::transform(expired.cbegin(), expired.cend(), std::back_inserter(cbs), 
        [](Timer::ptr timer) { return timer->cb_; });
    // check if need to repush to timer
    // TODO: low speed algorithm here
    timers_.erase(pos, timers_.end());
    timers_.insert(expired.cbegin(), expired.cend());
}

// add condition
void TimerManager::add_condition_timer(uint64_t ms, bool recurring, std::shared_ptr<void> cond, 
    std::function<void ()> cb, std::string name) {
    // use wrap func
    auto func_wrap = std::bind(&OnTimer, cond, cb);
    add_timer(ms, recurring, func_wrap, name);
}



}