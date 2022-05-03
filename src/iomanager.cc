#include "iomanager.h"
#include "fiber.h"
#include "scheduler.h"

#include <cerrno>
#include <cstddef>
#include <cstring>
#include <exception>
#include <memory>
#include <utility>
#include <sys/epoll.h>

namespace sylar {

IOManager::IOManager(size_t threads, bool use_caller, const std::string& name): 
Scheduler(threads, use_caller, name) {
    SYLAR_INFO("io manager start");
    // create epoll fd
    epfd_ = epoll_create1(EPOLL_CLOEXEC);

    // start scheduler
    start();
}

IOManager::~IOManager() {
    // close epoll fd
    close(epfd_);
}

// idle to wait more event
void IOManager::idle() {
    // epoll max events
    const uint64_t MAX_EVENTS = 256;
    epoll_event* events = new epoll_event[MAX_EVENTS];
    std::shared_ptr<epoll_event> shared_event(events, [](epoll_event* ptr) {
        delete [] ptr;
    });

    // epoll wait to monitor
    while (true) {
        // wait
        int count = epoll_wait(epfd_, events, MAX_EVENTS, -1);
        // check wait result
        // if errno is signal interrupt, ignore
        if (count < 0 && errno == EINTR) {
            continue;
        }
        // err happens, report err
        if (count < 0) {
            SYLAR_FMT_ERR("epoll wait failed, err: %s", strerror(errno));
            break;
        }
        // read filescriptor
        for (int index = 0; index < count; index++) {
            epoll_event& event = events[index];
            // fd context
            FdContext* fd_ctx = static_cast<FdContext*>(event.data.ptr);
            // check read event 
            if (event.events & (EPOLLIN | EPOLLHUP | EPOLLERR)) {
                fd_ctx->trigger_event(Event::READ);
            } 
            // check write event
            if (event.events & (EPOLLOUT | EPOLLHUP | EPOLLERR)) {
                fd_ctx->trigger_event(Event::WRITE);
            }
        }

        // should idle self
        auto idle_fiber = Fiber::get_this();
        idle_fiber->yield();
    }
}  

// add event and callback to fd 
void IOManager::FdContext::add_event(Event event, Scheduler::ptr sched, std::function<void()> cb) {
    // mutex
    MutexType::Lock lock(mutex_);
    // check if already exist
    auto pos = dispatcher.find(event);
    if (pos != dispatcher.end()) {
        SYLAR_FMT_DEBUG("event callback dont need to added, already exist, fd: %d, event: %d", fd, event);
        return;
    }
    // add event dispatcher
    dispatcher.insert(std::make_pair(event, EventContext::ptr(new EventContext(sched, cb))));
    SYLAR_FMT_DEBUG("event callback add successfully, fd: %d, event: %d", fd, event);
}

void IOManager::FdContext::del_event(Event event) {
    // mutex
    MutexType::Lock lock(mutex_);
    // check if exist
    auto pos = dispatcher.find(event);
    if (pos == dispatcher.end()) {
        SYLAR_FMT_DEBUG("event callback dont need to delete, not exist, fd: %d, event: %d", fd, event);
        return;
    }
    // delete from dispatcher
    dispatcher.erase(pos);
    SYLAR_FMT_DEBUG("event callback delete successfully, fd: %d, event: %d", fd, event);
}

// trigger event to call callback
void IOManager::FdContext::trigger_event(Event event) {
    // mutex
    MutexType::Lock lock(mutex_);
    try {
        // try to get context
        auto ctx = dispatcher.at(event);
        ctx->scheduler->schedule(ctx->fiber);
    } catch (std::exception& e) {
        SYLAR_FMT_ERR("unexpected event is triggered, fd: %d, event: %d", fd, event);
    }
}

// clear all event
void IOManager::FdContext::clear_event() {
    MutexType::Lock lock(mutex_);
    dispatcher.clear();
}


}