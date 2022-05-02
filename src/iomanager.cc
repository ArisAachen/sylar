#include "iomanager.h"
#include "scheduler.h"

#include <asm-generic/errno-base.h>
#include <cerrno>
#include <cstring>
#include <memory>
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
        int count = epoll_wait(epfd_, events, MAX_EVENTS, 0);
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
            // check event 
            if (event.events & (EPOLLERR | EPOLLHUP)) {
                
            }

        }
    }
}  





}