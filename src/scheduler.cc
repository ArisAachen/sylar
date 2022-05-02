#include "scheduler.h"
#include "fiber.h"
#include "log.h"
#include "mutex.h"
#include "thread.h"

#include <string>
#include <functional>

namespace sylar {

static thread_local Fiber::ptr schedule_fiber = nullptr;

Scheduler::Scheduler(size_t threads, bool use_caller, const std::string& name):
thread_num_(threads) , use_caller_(use_caller), name_(name) {
    // at least should create one thread
    SYLAR_ASSERT(thread_num_ > 0);
    // check if use main thread as fiber scheduler
    if (use_caller_) {
        thread_num_--;
        schedule_fiber = Fiber::ptr(new Fiber(std::bind(&Scheduler::run, this), 0, false, "Scheduler fiber"));
    }

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
    SYLAR_DEBUG("schedule destoried");
}

// start scheduler
void Scheduler::start() {
    // start log
    SYLAR_INFO("scheduler start");
    threads_.reserve(thread_num_);
    // begin to run thread
    for (auto thread : threads_) {
        thread->run();
        // store thread id 
        thread_ids_.push_back(thread->get_thread_id());
    }

    // check if use main thread as schedule
    if (use_caller_) {
        // create main fiber
        Fiber::get_this();
        // begin to schedule
        schedule_fiber->resume();
    }
}

void Scheduler::stop() {
    SYLAR_INFO("scheduler stop");
    // stop all thread
    for (auto thread : threads_) {
        // stop thread
        thread->stop();
    }
    // clear all running thread id
    thread_ids_.clear();
}

Fiber::ptr Scheduler::get_schedule_fiber() {
    return schedule_fiber;
}

void Scheduler::schedule(std::function<void ()> cb, int thr) {
    SYLAR_DEBUG("schedule add task");
    MutexType::Lock lock(mutex_);
    tasks_.emplace_back(new ScheduleTask(cb, use_caller_,thr));
    ConditionBlock::Block block(cond_);
    cond_.signal();
}

// run scheduler
void Scheduler::run() {
    SYLAR_INFO("scheduler run");
    // create idle fiber
    Fiber::get_this();

    while(true) {
        // check if is empty
        ScheduleTask::ptr task;
        {   
            // if all tasks execute end, should wait here
            MutexType::Lock lock(mutex_);
            SYLAR_FMT_DEBUG("current task size is %d", tasks_.size());
            if (tasks_.size() == 0) {
                // if use caller, directly return here
                if (use_caller_)
                    return;
                lock.unlock();
                Fiber::ptr idle_fiber(new Fiber(std::bind(&Scheduler::idle, this), 0, false, "idle"));
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