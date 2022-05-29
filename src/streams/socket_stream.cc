#include "socket_stream.h"
#include "../log.h"
#include <cstddef>

namespace sylar {

SocketStream::SocketStream(Socket::ptr sock, bool owner) 
    : sock_(sock), owner_(owner) {
    SYLAR_FMT_DEBUG("create socket stream, fd: %d", sock_->get_fd());
}

SocketStream::~SocketStream() {
    if (owner_ && sock_) 
        sock_->close();
    SYLAR_FMT_DEBUG("destory socket stream, fd: %d", sock_->get_fd());
}

int SocketStream::read(void* buf, size_t length) {
    // check if already connected
    if (is_connected()) 
        return -1;
    return sock_->recv(buf, length);
}

int SocketStream::read(ByteArray::ptr arr, size_t length) {
    if (is_connected()) 
        return -1;
    // TODO: add code here
    return 0;
}

int SocketStream::write(const void* buf, size_t length) {
    if (is_connected()) 
        return -1;
    return sock_->send(buf, length);
}

int SocketStream::write(const ByteArray::ptr arr, size_t length) {
    if (is_connected()) 
        return -1;
    // TODO: add code here
    return 0;
}

void SocketStream::close() {
    if (sock_)
        sock_->close();
}

Address::ptr SocketStream::get_remote_addr() {
    if (sock_)
        return sock_->get_remote_addr();
    return nullptr;
}

Address::ptr SocketStream::get_local_addr() {
    if (sock_)
        sock_->get_local_addr();
    return nullptr;
}


}