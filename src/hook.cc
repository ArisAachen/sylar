#include "hook.h"
#include "fiber.h"
#include "iomanager.h"
#include "log.h"
#include "timer.h"
#include "utils.h"
#include "fdmanager.h"

#include <cerrno>
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <memory>
#include <string>

#include <dlfcn.h>
#include <unistd.h>
#include <sys/socket.h>


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
    sylar::IOMgr::get_instance()->add_fd_event(fd, event, [fd, func, hook_name, &args...]() {
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
    SYLAR_FMT_DEBUG("socket add to fd manager successfully, fd: %d", fd);
    return fd;
}

int connect(int fd, const struct sockaddr *addr, socklen_t addrlen) {
    SYLAR_FMT_DEBUG("connect, fd: %d", fd);
    return connect_f(fd, addr, addrlen);
}

int accept(int fd, struct sockaddr *addr, socklen_t *addrlen) {
    int accept_fd = do_io(fd, accept_f, sylar::IOManager::Event::READ, "accept", addr, addrlen);
    if (accept_fd == -1) {
        SYLAR_FMT_ERR("accept failed, fd: %d, err: %s", fd, strerror(errno));
        return -1;
    }
    // need to hook
    if (sylar::SystemInfo::get_hook_enabled()) {
        sylar::FdMgr::get_instance()->add_fdctx(accept_fd);
        SYLAR_FMT_DEBUG("accept add to fd manager successfully, fd: %d, accept fd: %d", fd, accept_fd);
    }
    return accept_fd;
}

ssize_t read(int fd, void *buf, size_t bytes) {
    return do_io(fd, read_f, sylar::IOManager::Event::READ, "read", buf, bytes);
}

int close(int fd) {
    // check if need use hook here
    if (!sylar::SystemInfo::get_hook_enabled()) {
        SYLAR_FMT_DEBUG("close dont need use hook, fd: %d", fd);
        return close_f(fd);
    }
    int result = close_f(fd);
    if (result == -1)
        SYLAR_FMT_ERR("hook close fd failed, fd: %d, err: %s", fd, strerror(errno));
    sylar::FdMgr::get_instance()->del_fdctx(fd);
}

ssize_t readv(int fd, const struct iovec *iov, int iovcnt) {
    return do_io(fd, readv_f, sylar::IOManager::Event::READ, "readv", iov, iovcnt);
}

ssize_t recv(int fd, void *buf, size_t len, int flags) {
    return do_io(fd, recv_f, sylar::IOManager::Event::READ, "recv", buf, len, flags);
}

ssize_t recvfrom(int fd, void * buf, size_t len, int flags, struct sockaddr * addr, socklen_t *addrlen) {
    return do_io(fd, recvfrom_f, sylar::IOManager::Event::READ, "recvfrom", buf, len, 
        flags, addr, addrlen);
}

ssize_t recvmsg(int fd, struct msghdr * msg, int flags) {
    return do_io(fd, recvmsg_f, sylar::IOManager::Event::READ, "recvmsg", msg, flags);
}

ssize_t write(int fd, const void *buf, size_t nbyte) {
    return do_io(fd, write_f, sylar::IOManager::Event::WRITE, "write", buf, nbyte);
}

ssize_t writev(int fd, const struct iovec *iov, int iovcnt) {
    return do_io(fd, writev_f, sylar::IOManager::Event::WRITE, "writev", iov, iovcnt);
}

ssize_t send(int fd, const void * buf, size_t len, int flags) {
    return do_io(fd, send_f, sylar::IOManager::Event::WRITE, "send", buf, len, flags);
}

ssize_t sendto(int fd, const void * buf, size_t len, int flags, const struct sockaddr *addr, socklen_t addrlen) {
    return do_io(fd, sendto_f, sylar::IOManager::Event::WRITE, "sendto", buf, len, flags, addr, addrlen);
}

ssize_t sendmsg(int fd, const struct msghdr * msg, int flags) {
    return do_io(fd, sendmsg_f, sylar::IOManager::Event::WRITE, "sendmsg", msg, flags);
}



}

