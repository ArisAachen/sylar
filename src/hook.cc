#include "hook.h"
#include "fiber.h"
#include "iomanager.h"
#include "log.h"
#include "timer.h"
#include "utils.h"
#include "fdmanager.h"

#include <cerrno>
#include <cstdio>
#include <cstring>
#include <memory>
#include <string>

#include <dlfcn.h>

#define HOOK_FUNC(XX) \
    XX(sleep)   \
    XX(nanosleep)   \
    XX(socket)  \
    XX(connect) \
    XX(accept)  \
    XX(read)    \
    XX(readv)   \
    XX(recv)    \
    XX(recvfrom)    \
    XX(recvmsg) \
    XX(write)   \
    XX(writev)  \
    XX(send)    \
    XX(sendto)  \
    XX(sendmsg) \
    XX(close)


void hook_init() {
#define XX(name) name##_f = (name##_func)dlsym(RTLD_NEXT, #name);
    HOOK_FUNC(XX)
#undef XX
}

struct timer_info {
    bool cancelled {false};
};

template<typename OriginFunc, typename... Args>
static ssize_t do_io(int fd, OriginFunc func, sylar::IOManager::Event event, const std::string& hook_name, Args&&... args) {
    // check if need hook
    if (!sylar::SystemInfo::get_hook_enabled()) {
        SYLAR_FMT_DEBUG("do io op dont use hook, fd: %d, func name: %s", fd, hook_name.c_str());
        return func(fd, std::forward<Args>(args)...);
    }
    // try to add fd, make sure fd exist
    // sylar::FdMgr::get_instance()->add_fdctx(fd);
    auto ctx = sylar::FdMgr::get_instance()->get_fdctx(fd);
    // must not be empty
    if (ctx == nullptr) {
        SYLAR_FMT_ERR("do io op failed, fd is not yet created, fd: %d", fd);
        errno = EBADF;
        return -1;
    }
    // fd is not socket, or it is block, directly block here
    if (!ctx->is_socket() || !ctx->is_nonblock()) {
        SYLAR_FMT_DEBUG("use hook, but dont need to handle, fd: %d, socket: %d, nonblock: %d, func name: %s",
            fd, ctx->is_socket(), ctx->is_nonblock(), hook_name.c_str());
        return func(fd, std::forward<Args>(args)...);
    }
    // try to execute
    ssize_t count;
    do {
        count = func(fd, std::forward<Args>(args)...);
    } while(count == -1 && errno == EINTR);
    // socket is ready or some unexpected error happens
    if (count != -1 || errno != EAGAIN) {
        SYLAR_FMT_DEBUG("do io op has finished, fd: %d, count: %d, errno: %d, func name: %s",
            fd, count, errno, hook_name.c_str());
        return count;
    }
    // timer info 
    std::shared_ptr<timer_info> info_ptr(new timer_info);
    // timer
    sylar::Timer::ptr timer;
    // get timeout
    int timeout = ctx->get_timeout();
    // has timeout
    if (timeout != -1) {
        // add timer
        timer = sylar::IOMgr::get_instance()->add_condition_timer(timeout, false, info_ptr, [info_ptr, fd, event]() {
            // timeout already cancelled
            // that means io op has been execute successfully
            if (!info_ptr || info_ptr->cancelled)
                return;
            // delete event
            sylar::IOMgr::get_instance()->del_fd_event(fd, event);
        }, hook_name);
    }
    // TODO: callback, should optimize code here
    sylar::IOMgr::get_instance()->add_fd_event(fd, event, [fd, func, hook_name, args...]() {
        // use to execute found
        ssize_t count = func(fd, std::forward<Args>(args)...);
        // continue wait
        if (count == -1 && errno == EINTR) 
            return;
        // some error happens
        if (count == -1) {
            SYLAR_FMT_DEBUG("io event callback is failed, fd: %d, func name: %s, err: %s", fd, hook_name.c_str(), strerror(errno));
            return;
        }
        SYLAR_FMT_DEBUG("io event callback success, fd: %d, func name: %s", fd, hook_name.c_str());
    });
};


// extern c
extern "C" {

unsigned int sleep(unsigned int seconds) {
    // check if need use hook
    if (!sylar::SystemInfo::get_hook_enabled())
        return sleep_f(seconds);
    // get current fiber
    auto fiber = sylar::Fiber::get_this();
    sylar::IOMgr::get_instance()->add_timer(seconds * 1000, false, [fiber](){
        // wake up here
        fiber->resume();
    }, "sleep");
    // yield current to sleep
    fiber->yield();
    return 0;
}

int nanosleep(const struct timespec *req, struct timespec *rem) {
    if (!sylar::SystemInfo::get_hook_enabled()) 
        return nanosleep_f(req, rem);
    int timeout_ms = req->tv_sec * 1000 + req->tv_nsec / 1000 /1000;
    // get current fiber
    auto fiber = sylar::Fiber::get_this();
    sylar::IOMgr::get_instance()->add_timer(timeout_ms, false, [fiber](){
        // wake up here
        fiber->resume();
    }, "sleep");
    // yield current to sleep
    fiber->yield();
    return 0;    
}


int socket(int domain, int type, int protocol) {
    if (!sylar::SystemInfo::get_hook_enabled()) 
        return socket_f(domain, type, protocol);
    int fd = socket_f(domain, type, protocol);
    if (fd == -1) {
        SYLAR_FMT_ERR("socket failed, err: %s", strerror(errno));
        return -1;
    }
    // add to fd manager
    sylar::FdMgr::get_instance()->add_fdctx(fd);
    return fd;
}

}