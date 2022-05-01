#include "scheduler.h"
#include "fiber.h"
#include "log.h"
#include "mutex.h"
#include "thread.h"

#include <string>
#include <functional>

namespace sylar {


Scheduler::Scheduler(size_t threads, bool use_caller, const std::string& name):
thread_num_(threads) , use_caller_(use_caller), name_(name) {
    for (int index = 0; index < thread_num_; index++) {
        // create thread and run
        Thread::ptr thread(new Thread(std::bind(&Scheduler::run, this), "thread_" + std::to_string(index)));
        // push thread into vec
        threads_.push_back(thread);
    }
}

Scheduler::~Scheduler() {
    for (auto& thread : threads_) {
        thread->join();
    }
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

void Scheduler::schedule(std::function<void ()> cb, int thr) {
    SYLAR_INFO("schedule add task");
    MutexType::Lock lock(mutex_);
    tasks_.emplace_back(new ScheduleTask(cb, thr));
    ConditionBlock::Block block(cond_);
    cond_.signal();
}

// run scheduler
void Scheduler::run() {
    SYLAR_INFO("scheduler run");
    // create idle fiber
    Fiber::get_this();
    Fiber::ptr idle_fiber(new Fiber(std::bind(&Scheduler::idle, this), 0, false));

    while(true) {
        // check if is empty
        ScheduleTask::ptr task;
        {   
            // if all tasks execute end, should wait here
            MutexType::Lock lock(mutex_);
            if (tasks_.size() == 0) {
                lock.unlock();
                idle_fiber->resume();
            }

            // get task
            task = tasks_.front();
            tasks_.pop_front();
        }
        // execute fiber
        task->execute();
    }
}

void Scheduler::idle() {
    SYLAR_INFO("all tasks execute finished, idle scheduler");
    // wait here
    ConditionBlock::Block block(cond_);
    block.wait();
}


}