#include "address.h"
#include "hook.h"
#include "log.h"

#include <cstring>
#include <string>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

namespace sylar {



IPv4Address::ptr IPv4Address::create(const std::string &addr, uint16_t port) {
    return IPv4Address::ptr(new IPv4Address(addr, port));
}

// create ipv4 add by net type
IPv4Address::IPv4Address(uint32_t addr, uint16_t port) {
    // use net type addr directly
    addr_.sin_family = AF_INET;
    addr_.sin_addr.s_addr = addr;
    addr_.sin_port = port;
    SYLAR_FMT_DEBUG("create ipv4 addr success, addr: %s:%s", addr, port);
}

IPv4Address::IPv4Address(const std::string& msg, uint16_t port) {
    // try to convert 
    if(inet_pton(AF_INET, msg.c_str(), &addr_.sin_addr.s_addr) == -1) {
        SYLAR_FMT_ERR("convert addr message to ipv4 addr failed, addr: %s, err: %s", 
            msg.c_str(), strerror(errno));
        return;
    }
    // save family
    addr_.sin_family = AF_INET;
    // save port
    addr_.sin_port = htons(port);
    SYLAR_FMT_DEBUG("create ipv4 addr success, addr: %s:%s", msg.c_str(), port);
}

const sockaddr* IPv4Address::get_sockaddr() {
    return (sockaddr*) &addr_;
}

const sockaddr* IPv4Address::get_sockaddr() const {
    return (sockaddr*) &addr_;
}

// calculate ipv4 addr len
socklen_t IPv4Address::get_sockaddr_len() {
    return sizeof(addr_);
}

// convert net ipv4 to readable version
const std::string IPv4Address::to_string() {
    // create buf to store addr info
    int buflen = sizeof(addr_);
    char buf[buflen];
    // try to convert to readable ip info
    if (strcmp(inet_ntop(AF_INET, &addr_.sin_addr.s_addr, buf, buflen),"") == 0) {
        SYLAR_FMT_ERR("convert ipv4 addr to addr message failed, addr: %ld, err: %s", addr_.sin_addr.s_addr, 
            strerror(errno));
        return "";
    }
    // convert net port to host
    int port = ntohs(addr_.sin_port);
    std::string msg(buf);
    // format "127.0.0.1:8080"
    return msg + ":" + std::to_string(port);
} 
}