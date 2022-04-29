#include "thread.h"
#include <cstring>

namespace sylar {

// create thread
Thread::Thread(std::function<void()> cb, const std::string & name) :
    cb_(cb), name_(name) {}


// release thread
Thread::~Thread() {
    // check if thread is created
    if (thread_id_ == 0)
        return;
    // 
    int ret;
    int err = pthread_join(thread_id_, (void **)&ret);
    if (err != 0) 
        SYLAR_FMT_ERR("pthread join failed, err: %s", strerror(errno));
}

// run thread
void Thread::run() {
    int err = pthread_create(&thread_id_, nullptr, wrap, this);
    if (err != 0)
        SYLAR_FMT_ERR("create failed, thread name: %s, err: %s", name_.c_str(), strerror(errno));
}

// stop thread
void Thread::stop() {
    if (thread_id_ == 0) 
        return;
    pthread_exit(0);        
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
    if (err == 0) 
        SYLAR_FMT_ERR("set thread name failed, thread id: %d, error: %s", thread->thread_id_, strerror(errno));
    if (thread->cb_)
        thread->cb_();
    return (void*)1;
}

bool Thread::operator==(Thread & thread) {
    return pthread_equal(thread_id_, thread.thread_id_);
}


}