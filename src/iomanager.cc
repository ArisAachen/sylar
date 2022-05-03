#include "iomanager.h"
#include "fiber.h"
#include "log.h"
#include "macro.h"
#include "mutex.h"
#include "scheduler.h"

#include <cerrno>
#include <cstddef>
#include <cstdint>
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
            fd_ctx->trigger_event(epoll_to_event(event.events));
        }

        // should idle self
        auto idle_fiber = Fiber::get_this();
        idle_fiber->yield();
    }
}  

IOManager::Event IOManager::epoll_to_event(uint32_t ep_events) {
    int events;
    if (ep_events & (EPOLLIN | EPOLLHUP | EPOLLERR)) { 
        events |= int(Event::READ);
    } 
    if (ep_events & (EPOLLOUT | EPOLLHUP | EPOLLERR)) {
        events |= int(Event::WRITE);
    }
    return Event(events);
}

uint32_t IOManager::event_to_epoll(IOManager::Event events) {
    uint32_t ep_events;
    switch (events) {
    case Event::READ:
        ep_events = (EPOLLIN | EPOLLHUP | EPOLLERR);
        break;
    case Event::WRITE:
        ep_events = (EPOLLOUT | EPOLLHUP | EPOLLERR);
        break;
    case Event::RW:
        ep_events = (EPOLLIN | EPOLLOUT | EPOLLHUP | EPOLLERR);
        break;
    default:
        // never happens
        SYLAR_ASSERT(false);
    }
    return ep_events;
}

// add fd event
void IOManager::add_fd_event(int fd, Event event, std::function<void ()> cb) {
    // TODO: use vector as container has costs loss, should optimize here
    // TODO: also func void() is too simplify to cover all situation, should use template later
    // lock fd
    MutexType::Lock lock(mutex_);
    FdContext::ptr ctx;
    // try to find origin fd context
    for (int index = 0; index < fd_ctxs_.size(); index++) {
        if (fd_ctxs_[index]->fd == fd) {
            ctx = fd_ctxs_[index];
            break;
        }
    }
    // epoll op
    int op = EPOLL_CTL_MOD;
    // if not exist, create one
    if (ctx == nullptr) {
        ctx = FdContext::ptr(new FdContext);
        fd_ctxs_.push_back(ctx);
        op = EPOLL_CTL_ADD;
    }
    // add fd event to io manager
    ctx->add_event(event, shared_from_this(), cb);
    // operate epoll 
    struct epoll_event ep;
    ep.events = event_to_epoll(ctx->get_events());
    ep.data.ptr = ctx.get();
    int err = epoll_ctl(epfd_, op, fd, &ep);
    // it is ok, because signal will interrupt
    if (err == -1 && errno == EINTR) {
        return;
    }
    // err happens
    if (err == -1) {
        SYLAR_FMT_ERR("epoll add fd event failed, fd: %d, op: %d, event: %d, err: %s", fd, op, event, strerror(errno));
        return;
    }
    // success
    SYLAR_FMT_DEBUG("epoll add fd event successfully, fd: %d, op: %d, event: %d", fd, op, event);
}

void IOManager::del_fd_event(int fd, Event event) {
    MutexType::Lock lock(mutex_);
    FdContext::ptr ctx;
    // try to find origin fd context
    auto pos = fd_ctxs_.cbegin();
    for (; pos != fd_ctxs_.cend(); pos++) {
        if (pos->get()->fd == fd) {
            ctx = *pos;
            break;
        }
    }
    // check if context is exist
    if (ctx == nullptr) {
        SYLAR_FMT_DEBUG("fd dont need to be deleted, not exist, fd: %d", fd);
        return;
    }
    // delete event
    ctx->del_event(event);
    Event ev = ctx->get_events();
    // should delete
    int op = EPOLL_CTL_MOD;
    if (ev == Event::NONE) {
        // should delete pos
        fd_ctxs_.erase(pos);
        op = EPOLL_CTL_DEL;
    } 
    // operate epoll
    struct epoll_event ep;
    ep.events = event_to_epoll(ctx->get_events());
    ep.data.ptr = ctx.get();
    int err = epoll_ctl(epfd_, op, fd, &ep);
    // it is ok, because signal will interrupt
    if (err == -1 && errno == EINTR) {
        return;
    }
    // err happens
    if (err == -1) {
        SYLAR_FMT_ERR("epoll del fd event failed, fd: %d, op: %d, event: %d, err: %s", fd, op, event, strerror(errno));
        return;
    }
    // success
    SYLAR_FMT_DEBUG("epoll del fd event successully, fd: %d, op: %d, event: %d", fd, op, event);
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

// get all current events
IOManager::Event IOManager::FdContext::get_events() {
    // mutex
    MutexType::Lock lock(mutex_);
    // append events
    int events;
    for (auto& item : dispatcher)
        events |= int(item.first);
    return Event(events);
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