#ifndef __SYLAR_SRC_HOOK_H__
#define __SYLAR_SRC_HOOK_H__

#include <cstddef>
#include <cstdio>

#include <time.h>
#include <unistd.h>
#include <sys/socket.h>

extern "C" {

typedef unsigned int (*sleep_func)(unsigned int seconds);
extern sleep_func sleep_f;

typedef int (*nanosleep_func)(const struct timespec *req, const struct timespec *rem);
extern nanosleep_func nanosleep_f;

typedef int (*socket_func)(int domain, int type, int protocol);
extern socket_func socket_f;

typedef int (*connect_func)(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
extern connect_func connect_f;

typedef int (*accept_func)(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
extern accept_func accept_f;

typedef ssize_t (*read_func)(int sockfd, void *buf, size_t count);
extern read_func read_f;

typedef ssize_t (*readv_func)(int fd, const struct iovec *iov, int iovcnt);
extern readv_func readv_f;

typedef ssize_t (*recv_func)(int fd, void *buf, size_t len, int flags);
extern recv_func recv_f;

typedef ssize_t (*recvfrom_func)(int fd, void *buf, size_t len, int flags, struct sockaddr *src_addr, socklen_t *addrlen);
extern recvfrom_func recvfrom_f;

typedef ssize_t (*recvmsg_func)(int fd, struct msghdr *msg, int flags);
extern recvmsg_func recvmsg_f;

typedef ssize_t (*write_func)(int fd, const void *buf, size_t count);
extern write_func write_f;

typedef ssize_t (*writev_func)(int fd, const struct iovec *iov, int iovcnt);
extern writev_func writev_f;

typedef ssize_t (*send_func)(int fd, const void* buf, size_t len, int flags);
extern send_func send_f;

typedef ssize_t (*sendto_func)(int fd, const void* buf, size_t len, int flags, struct sockaddr *addr, socklen_t addrlen);
extern sendto_func sendto_f;

typedef ssize_t (*sendmsg_func)(int fd, struct msghdr *mdg, int flags);
extern sendmsg_func sendmsg_f;

typedef int (*close_func)(int fd);
extern close_func close_f;

}

#endif