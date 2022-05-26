#include "address.h"
#include "log.h"
#include "socket.h"
#include "tcp_server.h"

#include <functional>
#include <vector>

namespace sylar {

TcpServer::TcpServer(const std::string& name, int type, IOManager::ptr io_worker, 
    IOManager::ptr accept_worker):
    name_(name), type_(type), io_worker_(io_worker), accept_worker_(accept_worker) {
    SYLAR_FMT_DEBUG("create tcp server, name: %s", name_.c_str());
}

TcpServer::~TcpServer() {
    for (auto& sock : sockets_) 
        sock->close();
    sockets_.clear();
}

bool TcpServer::bind(Address::ptr addr) {
    std::vector<Address::ptr> addrs {addr};
    std::vector<Address::ptr> fails;
    return bind(addrs, fails);
}

bool TcpServer::bind(const std::vector<Address::ptr> addrs, std::vector<Address::ptr> fails) {
    // bind all server
    for (auto addr : addrs) {
        // create tcp socket
        Socket::ptr sock = Socket::create_tcp(addr);
        // bind addr
        if (!sock->bind(addr)) {
            fails.push_back(addr);
            continue;
        }
        // listen socket
        if (!sock->listen()) {
            fails.push_back(addr);
            continue;
        }
        SYLAR_FMT_DEBUG("create tcp socket successfully, fd: %d", sock->get_fd());
        sockets_.push_back(sock);
    }
    // if exist at least one socket failed
    // should close all
    if (!fails.empty()) {
        sockets_.clear();
        return false;
    }
    return true;
}

bool TcpServer::start() {
    // check if server is running
    if (running_) {
        SYLAR_WARN("tcp server is already running");
        return false;
    }
    running_ = true;
    for (auto sock : sockets_)
        accept_worker_->schedule(std::bind(&TcpServer::start_accept, this, sock));
    SYLAR_DEBUG("tcp server start");
    return true;
}

bool TcpServer::stop() {
    if (!running_) {
        SYLAR_WARN("tcp server is already stopped");
        return false;
    }
    for (auto sock : sockets_) {
        sock->cancel_all();
        sock->close();
    }
    SYLAR_DEBUG("tcp server stop");
    return true;
}

void TcpServer::handle_client(Socket::ptr sock) {
    SYLAR_FMT_DEBUG("handle client is called, fd: %d", sock->get_fd());
}

void TcpServer::start_accept(Socket::ptr sock) {
    while (running_) {
        // accept
        Socket::ptr client = sock->accept();
        if (client) {
            io_worker_->schedule(std::bind(&TcpServer::handle_client, this, client));
        }
    }
}

}
