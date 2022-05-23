#include "socket.h"
#include "address.h"
#include "iomanager.h"
#include "log.h"
#include "utils.h"

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

bool Socket::send(void* buf, size_t len, int flags) {
    // use send must make sure socket is connected
    if (!connected_) {
        SYLAR_FMT_ERR("send buf failed, fd: %d, err: %s", fd_, "not connected yet");
        return false;
    }
    if (::send(fd_, buf, len, flags) == -1) {
        SYLAR_FMT_ERR("send buf failed, fd: %d, err: %s", fd_, strerror(errno));
        return false;
    }
    SYLAR_FMT_DEBUG("send buf success, fd: %d", fd_);
    return true;    
}

bool Socket::send(iovec *vec, size_t count, int flags) {
    // use send must make sure socket is connected
    if (!connected_) {
        SYLAR_FMT_ERR("send vec failed, fd: %d, err: %s", fd_, "not connected yet");
        return false;
    }
    if (::send(fd_, vec, count, flags) == -1) {
        SYLAR_FMT_ERR("send vec failed, fd: %d, err: %s", fd_, strerror(errno));
        return false;
    } 
    SYLAR_FMT_DEBUG("send vec success, fd: %d", fd_);
    return true;
}

bool Socket::send_to(void* buf, size_t len, Address::ptr addr, int flags) {
    if (::sendto(fd_, buf, len, flags, addr->get_sockaddr(), addr->get_sockaddr_len()) == -1) {
        SYLAR_FMT_ERR("sendto buf failed, fd: %d, err: %s", fd_, strerror(errno));
        return false;
    } 
    SYLAR_FMT_DEBUG("sendto buf success, fd: %d", fd_);
    return true;    
}

bool Socket::send_to(iovec *vec, int count, Address::ptr addr, int flags) {
    if (::sendto(fd_, vec, count, flags, addr->get_sockaddr(), addr->get_sockaddr_len()) == -1) {
        SYLAR_FMT_ERR("sendto vec failed, fd: %d, err: %s", fd_, strerror(errno));
        return false;
    } 
    SYLAR_FMT_DEBUG("sendto vec success, fd: %d", fd_);
    return true;
}

bool Socket::recv(void* buf, size_t len, int flags) {
    // use send must make sure socket is connected
    if (!connected_) {
        SYLAR_FMT_ERR("recv buf failed, fd: %d, err: %s", fd_, "not connected yet");
        return false;
    }
    if (::recv(fd_, buf, len, flags) == -1) {
        SYLAR_FMT_ERR("send buf failed, fd: %d, err: %s", fd_, strerror(errno));
        return false;
    }
    SYLAR_FMT_DEBUG("send buf success, fd: %d", fd_);
    return true;
}

bool Socket::recv(iovec* buf, size_t count, int flags) {
    // use send must make sure socket is connected
    if (!connected_) {
        SYLAR_FMT_ERR("recv vec failed, fd: %d, err: %s", fd_, "not connected yet");
        return false;
    }
    if (::recv(fd_, buf, count, flags) == -1) {
        SYLAR_FMT_ERR("recv vec failed, fd: %d, err: %s", fd_, strerror(errno));
        return false;
    }
    SYLAR_FMT_DEBUG("recv vec success, fd: %d", fd_);
    return true;    
}

bool Socket::recv_from(void *buf, size_t len, Address::ptr addr, int flags) {
    // TODO: recvfrom func require sock_len size
    if (::recvfrom(fd_, buf, len, flags, addr->get_sockaddr(), nullptr) == -1) {
        SYLAR_FMT_ERR("sendto buf failed, fd: %d, err: %s", fd_, strerror(errno));
        return false;
    }
    SYLAR_FMT_DEBUG("recvfrom buf success, fd: %d", fd_);
    return true;    
}

bool Socket::recv_from(iovec *buf, size_t count, Address::ptr addr, int flags) {
    // TODO: recvfrom func require sock_len size
    if (::recvfrom(fd_, buf, count, flags, addr->get_sockaddr(), nullptr) == -1) {
        SYLAR_FMT_ERR("sendto vec failed, fd: %d, err: %s", fd_, strerror(errno));
        return false;
    }
    SYLAR_FMT_DEBUG("recvfrom vec success, fd: %d", fd_);
    return true;        
}

void Socket::cancel_read() {
    // check if need to del
    if (SystemInfo::get_hook_enabled()) 
        IOMgr::get_instance()->del_fd_event(fd_, IOManager::Event::READ);
}

void Socket::cancel_write() {
    // check if need to del
    if (SystemInfo::get_hook_enabled()) 
        IOMgr::get_instance()->del_fd_event(fd_, IOManager::Event::WRITE);
}

void Socket::cancel_all() {
    // check if need to del
    if (SystemInfo::get_hook_enabled()) {
        IOMgr::get_instance()->del_fd_event(fd_, IOManager::Event::READ);
        IOMgr::get_instance()->del_fd_event(fd_, IOManager::Event::WRITE);
    }
}

}