#include "socket.h"
#include "address.h"
#include "log.h"

#include <cstring>
#include <sys/socket.h>

namespace sylar {

// create tcp socket
Socket::ptr Socket::create_tcp(Address::ptr addr) {
    return Socket::ptr(new Socket(addr->get_family(), Type::TCP, IPPROTO_TCP));
}

// create udp socket
Socket::ptr Socket::create_udp(Address::ptr addr) {
    return Socket::ptr(new Socket(addr->get_family(), Type::UDP, IPPROTO_UDP));
}

// create default ipv4 socket
Socket::ptr Socket::create_tcp_socket() {
    return Socket::ptr(new Socket(AF_INET, Type::TCP, IPPROTO_TCP));
}

// create default ipv4 udp 
Socket::ptr Socket::create_udp_socket() {
    return Socket::ptr(new Socket(AF_INET, Type::UDP, IPPROTO_UDP));
}

// create default ipv6 tdp socket
Socket::ptr Socket::create_tcp_socket6() {
    return Socket::ptr(new Socket(AF_INET6, Type::TCP, IPPROTO_TCP));
}

Socket::ptr Socket::create_udp_socket6() {
    return Socket::ptr(new Socket(AF_INET6, Type::UDP, IPPROTO_UDP));
}

Socket::Socket(int family, int type, int protocol):
family_(family), type_(type), protocol_(protocol) {
    // TODO: maybe dont need to call socket func here
    // create socket
    int fd_ = socket(family, family, protocol);
    // should check if create success here
    if (fd_ == -1) {
        SYLAR_FMT_ERR("create socket failed, family: %d, type: %d, protocol: %d, err: %s",
            family, type, protocol, strerror(errno));
        return;
    }
    SYLAR_FMT_DEBUG("create sock success, fd: %d", fd_);
} 

// socket accept
Socket::ptr Socket::accept() {
    struct sockaddr addr;
    socklen_t len;
    int accept_fd = ::accept(fd_, &addr, &len);
    if (accept_fd == -1) {
        SYLAR_FMT_ERR("accept socket failed, fd: %d, err: %s", fd_, strerror(errno));
        return nullptr;
    }
    // create socket obj
    Socket::ptr sd;
    sd->family_ = addr.sa_family;
    sd->type_ = Type::TCP;
    sd->protocol_ = IPPROTO_TCP;
    sd->remote_addr_ = Address::create(&addr, len);
    SYLAR_FMT_DEBUG("accept from remote success, fd: %d, remote addr: %s",
        accept_fd, sd->remote_addr_->to_string().c_str());
    return sd;
}

// as server, socket should bind first
bool Socket::bind(Address::ptr addr) {
    // try to bind server socket
    if (::bind(fd_, addr->get_sockaddr(), addr->get_sockaddr_len()) == -1) {
        SYLAR_FMT_ERR("bind socket failed, fd: %d, err: %s", fd_, strerror(errno));
        return false;
    }
    // should save local addr info here
    local_addr_ = addr;
    SYLAR_FMT_DEBUG("bind socket success, fd: %d", fd_);
    return true;
}

bool Socket::listen(int count) {
    // try to listen socket from remote
    if (::listen(fd_, count) == -1) {
        SYLAR_FMT_ERR("listen socket failed, fd: %d, err: %s", fd_, strerror(errno));
        return false;
    }
    // one listen is called successfully, 
    // means server has start
    connected_ = true;
    SYLAR_FMT_DEBUG("accept socket success, fd: %d", fd_);
    return true;
}

// client try to connect to server
bool Socket::connect(Address::ptr addr) {
    // connect to server
    if (::connect(fd_, addr->get_sockaddr(), addr->get_sockaddr_len()) == -1) {
        SYLAR_FMT_ERR("connect to server failed, fd: %d, server: %s, err: %s", 
            fd_, addr->to_string().c_str(), strerror(errno));
        return false;
    }
    connected_ = true;
    SYLAR_FMT_DEBUG("connect socket success, fd: %d, server: %s", fd_, addr->to_string().c_str());
    return true;
}

}