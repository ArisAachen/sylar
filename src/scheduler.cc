#include "scheduler.h"
#include "fiber.h"
#include "log.h"
#include "mutex.h"
#include "thread.h"

#include <cstddef>
#include <memory>
#include <string>
#include <functional>
#include <unistd.h>

namespace sylar {

/// thread schedule diber
static thread_local Fiber::ptr schedule_fiber = nullptr;
/// indicate if current is main thread as scheduler
static thread_local bool is_scheduler = false;

Scheduler::Scheduler(size_t threads, bool use_caller, const std::string& name):
thread_num_(threads) , use_caller_(use_caller), name_(name) {
    // at least should create one thread
    SYLAR_ASSERT(thread_num_ > 0);
    // check if use main thread as fiber scheduler
    if (use_caller_) {
        is_scheduler = true;
        thread_num_--;
        schedule_fiber = Fiber::ptr(new Fiber(std::bind(&Scheduler::run, this), 0, false, "Scheduler fiber"));
    }

    for (int index = 0; index < thread_num_; index++) {
        // create thread and run
        Thread::ptr thread(new Thread(std::bind(&Scheduler::run, this), "thread_" + std::to_string(index)));
        // push thread into vec
        threads_.push_back(thread);
    }
    SYLAR_FMT_DEBUG("scheduler create, scheduler name: %s, user caller: %d, thread count: %d", name_.c_str(), use_caller_, thread_num_);
}

Scheduler::~Scheduler() {
    // SYLAR_DEBUG("schedule destoried");
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
        is_scheduler = true;
        // create main fiber
        Fiber::get_this();
        // begin to schedule
        schedule_fiber->resume();
    }

    // join to wait here
    for (auto& thread : threads_) {
        thread->join();
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

bool Scheduler::is_scheduler_fiber() {
    return is_scheduler;
}

Fiber::ptr Scheduler::get_schedule_fiber() {
    return schedule_fiber;
}

void Scheduler::schedule(std::function<void ()> cb, int thr) {
    // SYLAR_DEBUG("schedule add task");
    task_push(ScheduleTask::ptr(new ScheduleTask(cb, use_caller_,thr)));
}

void Scheduler::schedule(Fiber::ptr fiber, int thr) {
    // SYLAR_DEBUG("schedule add task");
    task_push(ScheduleTask::ptr(new ScheduleTask(fiber, thr)));
}

// run scheduler
void Scheduler::run() {
    SYLAR_INFO("scheduler run");
    // create idle fiber
    Fiber::get_this();

    while(true) {
        // check if is empty
        ScheduleTask::ptr task = task_pop();
        // std::weak_ptr<ScheduleTask> task_weak(task);
        // SYLAR_FMT_DEBUG(">>>>>> schedule task, expired: %d, use count: %d", task_weak.expired(), task_weak.use_count());
        if (task != nullptr)
            task->execute();
    }
    SYLAR_INFO("scheduler stop");
}

void Scheduler::idle() {
    SYLAR_INFO("all tasks execute finished, idle scheduler");
    // wait here
    ConditionBlock::Block block(cond_);
    block.wait();
    // SYLAR_DEBUG("at least one task is added, exit idle");
}

bool Scheduler::is_tasks_empty() {
    MutexType::Lock lock(mutex_);
    return tasks_.empty();
}

void Scheduler::task_push(ScheduleTask::ptr task) {
    MutexType::Lock lock(mutex_);
    tasks_.push_back(std::move(task));
    ConditionBlock::Block block(cond_);
    block.signal();
}

Scheduler::ScheduleTask::ptr Scheduler::task_pop() {
    // check if tasks is empty, if is should call idle to wait
    if (is_tasks_empty()) {
        Fiber::ptr idle_fiber(new Fiber(std::bind(&Scheduler::idle, this), 0, use_caller_, "idle"));
        idle_fiber->resume();
    }
    // if is not empty， pop one elem
    MutexType::Lock lock(mutex_);
    if (!tasks_.empty()) {
        ScheduleTask::ptr task = tasks_.front();
        tasks_.pop_front();
        return task;
    }
    SYLAR_WARN("non task is poped");
    return nullptr;
}

}