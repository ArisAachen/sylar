#include "fdmanager.h"
#include "log.h"



#include <cerrno>
#include <cstdio>
#include <cstring>
#include <algorithm>

#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/fcntl.h>

namespace sylar {

FdCtx::FdCtx(int fd): fd_(fd) {
    // create and init
    init();
    // log
    SYLAR_FMT_DEBUG("create fd context obj, fd: %d", fd);
}


FdCtx::~FdCtx() {
    if (fd_ != 0)
        close(fd_);
    SYLAR_FMT_DEBUG("fd obj has been destoried, fd: %d", fd_);
}

void FdCtx::init() {
    struct stat file_stat;
    // check file stat
    if (fstat(fd_, &file_stat) == -1) {
        SYLAR_FMT_ERR("check fd stat failed, fd: %d, err: %s", fd_, strerror(errno));
        return;
    }
    // check if fule is socket
    is_socket_ = S_ISSOCK(file_stat.st_mode);
    // if fd is socket type, need set non-block state
    if (is_socket_) {
        set_nonblock(true);
    }
}

// set fd non block state
void FdCtx::set_nonblock(bool nonblock) {
    int flags = fcntl(fd_, F_GETFD);
    if (flags == -1) {
        SYLAR_FMT_ERR("get fd stat failed, fd: %d, err: %s", fd_, strerror(errno));
        return;
    }
    // set nonblock
    flags = nonblock ? (flags|O_NONBLOCK) : (flags&O_NONBLOCK);
    if (fcntl(fd_, F_SETFD, flags) == -1) {
        SYLAR_FMT_ERR("set fd stat failed, fd: %d, err: %s", fd_, strerror(errno));
        return;        
    }
    is_nonblock_ = nonblock;
    SYLAR_FMT_DEBUG("set fd non-block success, fd: %d, nonblock: %d", fd_, nonblock);
}

// set timeout
void FdCtx::set_timeout(int timeout) {
    // only socket type allow to set timeout
    if (!is_socket_) {
        SYLAR_FMT_ERR("only socket type allow set timeout, fd: %d", fd_);
        return;
    }
    // timeout seconds
    struct timeval val = {
        .tv_sec = timeout,
        .tv_usec = 0,
    };
    // set socket rcv timeout
    if (setsockopt(fd_, SOL_SOCKET, SO_RCVTIMEO, &val, sizeof(val)) == -1) {
        SYLAR_FMT_ERR("set fd rcv timeout failed, fd: %d, err: %s", fd_, strerror(errno));
        return;
    }
    // set socket snd timeout
    if (setsockopt(fd_, SOL_SOCKET, SO_SNDTIMEO, &val, sizeof(val)) == -1) {
        SYLAR_FMT_ERR("set fd snd timeout failed, fd: %d, err: %s", fd_, strerror(errno));
        return;
    }
    // set timeout
    SYLAR_FMT_DEBUG("set fd timeout success, fd: %d, timeout: %ds", fd_, timeout);
}


FdManager::FdManager() {
    // resize to 64 
    fd_vec_.reserve(64);
}

FdManager::~FdManager() {
    fd_vec_.clear();
}

// add fd
void FdManager::add_fdctx(int fd) {
    if (get_fdctx(fd) != nullptr) {
        SYLAR_FMT_DEBUG("dont need to add fd, already exist, fd: %d", fd);
        return;
    }
    // mutex 
    MutexType::WriteLock lock(mutex_);
    fd_vec_.emplace_back(new FdCtx(fd));
    SYLAR_FMT_DEBUG("add fd to manager success: %d", fd);
}

// delete fd context
void FdManager::del_fdctx(int fd) {
    // remove context
    MutexType::WriteLock lock(mutex_);
    auto pos = std::remove_if(fd_vec_.begin(), fd_vec_.end(), [fd](FdCtx::ptr ctx) {
        return ctx->get_fd() == fd;
    });
    fd_vec_.erase(pos, fd_vec_.end());
}

FdCtx::ptr FdManager::get_fdctx(int fd) {
    MutexType::ReadLock lock(mutex_);
    auto pos = std::find_if(fd_vec_.begin(), fd_vec_.end(), [fd](FdCtx::ptr ctx) {
        return ctx->get_fd() == fd;
    });
    // not found
    if (pos == fd_vec_.end()) {
        SYLAR_FMT_WARN("fd cannnot be found in manager, fd: %d", fd);
        return nullptr;
    }
    // found
    return *pos;
}

}