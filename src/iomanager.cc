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
#include <memory>
#include <utility>
#include <exception>


#include <fcntl.h>
#include <unistd.h>
#include <sys/epoll.h>

namespace sylar {

IOManager::IOManager(size_t threads, bool use_caller, const std::string& name): 
Scheduler(threads, use_caller, name) {
    SYLAR_INFO("io manager create");
    // create epoll fd
    epfd_ = epoll_create1(EPOLL_CLOEXEC);
}

IOManager::~IOManager() {
    SYLAR_INFO("io manager destoried");
    // close epoll fd
    close(epfd_);
}

// idle to wait more event
void IOManager::idle() {
    SYLAR_DEBUG("io manager idle start");
    // epoll max events
    const uint64_t MAX_EVENTS = 256;
    epoll_event* events = new epoll_event[MAX_EVENTS];
    std::shared_ptr<epoll_event> shared_event(events, [](epoll_event* ptr) {
        delete [] ptr;
    });

    // wait
    SYLAR_DEBUG("prepare to epoll wait");
    
    int count = epoll_wait(epfd_, events, MAX_EVENTS, 5 * 1000);
    SYLAR_DEBUG("end to epoll wait");
    // check wait result
    // if errno is signal interrupt, ignore
    if (count < 0 && errno == EINTR) {
        return;
    }
    // err happens, report err
    if (count < 0) {
        SYLAR_FMT_ERR("epoll wait failed, err: %s", strerror(errno));
        return;
    }
    // read filescriptor
    for (int index = 0; index < count; index++) {
        epoll_event& event = events[index];
        // fd context
        FdContext* fd_ctx = static_cast<FdContext*>(event.data.ptr);
        if (fd_ctx == nullptr) {
            SYLAR_ERR("fd context convert failed");
            continue;
        }
        fd_ctx->trigger_event(epoll_to_event(event.events));
    }

    SYLAR_INFO("io manager idle end");
}  

IOManager::Event IOManager::epoll_to_event(uint32_t ep_events) {
    int events = 0;
    if (ep_events & EPOLLIN) {
        return Event::READ;
    } 
    if (ep_events & EPOLLOUT ) {
        return Event::WRITE;
    }
    return Event::NONE;
}

uint32_t IOManager::event_to_epoll(IOManager::Event events) {
    uint32_t ep_events;
    switch (events) {
    case Event::READ:
        ep_events = EPOLLIN;
        break;
    case Event::WRITE:
        ep_events = EPOLLOUT;
        break;
    case Event::RW:
        ep_events = EPOLLIN | EPOLLOUT;
        break;
    default:
        // never happens
        SYLAR_ASSERT(false);
    }
    return ep_events;
}

int IOManager::get_backend_fd() {
    return epfd_;
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
        // set non-block
        int flags = fcntl(fd, F_GETFL);
        flags |= O_NONBLOCK;
        int err = fcntl(fd, F_SETFL, flags);
        if (err == -1) {
            SYLAR_FMT_ERR("set fd non-block failed, fd: %d, err: %s", fd, strerror(errno));
        }
        // set fd context
        ctx = FdContext::ptr(new FdContext);
        ctx->fd = fd;
        fd_ctxs_.push_back(ctx);
        op = EPOLL_CTL_ADD;
    }
    // add fd event to io manager
    ctx->add_event(event, shared_from_this(), cb);
    // operate epoll 
    struct epoll_event ep;
    ep.events = EPOLLIN | EPOLLET;
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
    ep.events = EPOLLIN;
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
    if (op == EPOLL_CTL_DEL) {
        close(fd);
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
    // TODO: here should consider all events
    for (auto& item : dispatcher) {
        return item.first;
    }
    return Event::NONE;
}

// trigger event to call callback
void IOManager::FdContext::trigger_event(Event event) {
    // mutex
    MutexType::Lock lock(mutex_);
    try {
        // try to get context
        auto ctx = dispatcher.at(event);
        auto scheduler = ctx->scheduler.lock();
        if (scheduler) {
            Fiber::ptr fiber(new Fiber(ctx->cb, 0, is_scheduler_fiber(), "fd fiber"));
            scheduler->schedule(fiber);
        }
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