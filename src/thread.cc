#include "thread.h"
#include "log.h"
#include <cstring>
#include <pthread.h>

namespace sylar {

// create thread
Thread::Thread(std::function<void()> cb, const std::string & name) :
    cb_(cb), name_(name) {}


// release thread
Thread::~Thread() {
}

// run thread
void Thread::run() {
    // check if thread is already in running state
    if (running_) 
        return;
    // create thread
    int err = pthread_create(&thread_id_, nullptr, wrap, this);
    if (err != 0)
        SYLAR_FMT_ERR("create failed, thread name: %s, err: %s", name_.c_str(), strerror(errno));
    running_ = true;
    SYLAR_FMT_INFO("create thread id: %lld", thread_id_);
}

// stop thread
void Thread::stop() {
    if (thread_id_ == 0 || !running_) 
        return;
    running_ = false;
    pthread_exit(0);
}

void Thread::join() {
    // check if thread is already in running state
    if (!running_) 
        return;
    int ret = pthread_join(thread_id_, nullptr);
    if (ret != 0)
        SYLAR_FMT_ERR("pthread join failed, thread id: %d, err: %s", thread_id_, strerror(errno));
    SYLAR_DEBUG("pthread join end");
}

// swap func
void Thread::reset(std::function<void(void)>cb, const std::string & name) {
    // stop origin thread
    stop();
    // reset 
    cb_ = cb;
    name_ = name;
}

void *Thread::wrap(void *arg) {
    // convert 
    auto thread =  static_cast<Thread*>(arg);
    if (thread == nullptr) {
        SYLAR_FMT_ERR("convert thread failed, %s", "static cast");
        return (void*)1;
    }
    // set thread name
    int err = pthread_setname_np(thread->thread_id_, thread->name_.c_str());
    if (err != 0) 
        SYLAR_FMT_ERR("set thread name failed, thread id: %d, error: %s", thread->thread_id_, strerror(errno));
    if (thread->cb_)
        thread->cb_();
    return (void*)1;
}

bool Thread::operator==(Thread & thread) {
    return pthread_equal(thread_id_, thread.thread_id_);
}


}