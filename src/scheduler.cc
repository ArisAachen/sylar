#include "scheduler.h"
#include "log.h"
#include "thread.h"

#include <functional>
#include <string>


namespace sylar {


Scheduler::Scheduler(size_t threads, bool use_caller, const std::string& name):
thread_num_(threads) , use_caller_(use_caller), name_(name) {
    for (int index = 0; index < thread_num_; thread_num_++) {
        // create thread and run
        Thread::ptr thread(new Thread(std::bind(&Scheduler::run, this), "thread_" + std::to_string(index)));
        thread->run();
        // store thread id
        thread_ids_.push_back(thread->get_thread_id());
        // push thread into vec
        threads_.push_back(thread);
    }
}

Scheduler::~Scheduler() {

}

// start scheduler
void Scheduler::start() {
    // start log
    SYLAR_INFO("scheduler start");
    /// begin to lock
    MutexType::Lock lock(mutex_);
    threads_.reserve(thread_num_);
    // begin to run thread
    for (auto thread : threads_) {
        thread->run();
        // store thread id 
        thread_ids_.push_back(thread->get_thread_id());
    }
}

void Scheduler::stop() {
    SYLAR_INFO("scheduler stop");
    MutexType::Lock lock(mutex_);
    // stop all thread
    for (auto thread : threads_) {
        // stop thread
        thread->stop();
    }
    // clear all running thread id
    thread_ids_.clear();
}

// run scheduler
void Scheduler::run() {
    
}

}