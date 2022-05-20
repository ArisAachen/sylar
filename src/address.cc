#include "address.h"
#include "hook.h"
#include "log.h"
#include "macro.h"

#include <vector>
#include <cstddef>
#include <string>
#include <cstring>
#include <utility>

#include <netdb.h>
#include <ifaddrs.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>


namespace sylar {

bool Address::get_ip_info(std::map<std::string, std::vector<Address::ptr>> ip_map, 
    int family) {
    struct ifaddrs *next, *result;
    // try to get addr info 
    if (getifaddrs(&result) == -1) {
        SYLAR_FMT_ERR("get ip addr info failed, err: %s", strerror(errno));
        return false;
    }
    // all addr info
    for (next = result; next; next = result->ifa_next) {
        // if family match with expected one
        if (family != AF_UNSPEC && family != next->ifa_addr->sa_family) 
            continue;
        // try to get, if not exist, should create automatic
        std::vector<Address::ptr> &ref_vec = ip_map[next->ifa_name];
        // push back
        Address::ptr addr = Address::create(next->ifa_addr, next->ifa_addr->sa_len);
        SYLAR_FMT_DEBUG("ip addr: %s", addr->to_string().c_str());
        ref_vec.push_back(std::move(addr));
    }
    // free addrs 
    freeifaddrs(result);
    return true;
}

Address::ptr Address::create(const sockaddr *addr, socklen_t len) {
    switch (addr->sa_family) {
    case AF_INET:
        return Address::ptr(new IPv4Address(*(sockaddr_in*)addr));
    case AF_INET6:
        return Address::ptr(new IPv6Address(*(sockaddr_in6*)addr));
    default:
        SYLAR_ASSERT(false);
    }
    return nullptr;
}

bool Address::look_up(std::vector<Address::ptr> &result, 
    const std::string &host, int family, int type, int protocol) {
    // indicate request info
    struct addrinfo info, *info_list;
    // memset all memory
    memset(&info, 0, sizeof(info));
    info.ai_family = family;
    info.ai_socktype = type;
    info.ai_protocol = protocol;
    // begin to request 
    int err = getaddrinfo(host.c_str(), nullptr, &info, &info_list);
    if (err != 0) {
        SYLAR_FMT_ERR("look up host failed, host: %s, family: %d, type: %d, protocol: %d, err: %s", 
            host.c_str(), family, type, protocol, gai_strerror(err));
        return false;
    }
    // try to add to vec
    for (struct addrinfo* pre = info_list; pre != nullptr; pre = pre->ai_next) {
        // create address obj
        Address::ptr addr = Address::create(pre->ai_addr, pre->ai_addrlen);
        SYLAR_FMT_DEBUG("parse host success, host: %s, addr: %s", host.c_str(), 
            addr->to_string().c_str());
        result.push_back(std::move(addr));
    }
    freeaddrinfo(info_list);
    return true;
}

IPv4Address::IPv4Address(const sockaddr_in& addr) {
    addr_ = addr;
    SYLAR_FMT_DEBUG("create ipv4 addr success, addr: %s:%d", addr.sin_addr, ntohs(addr.sin_port));
}

// create ipv4 add by net type
IPv4Address::IPv4Address(uint32_t addr, uint16_t port) {
    // use net type addr directly
    addr_.sin_family = AF_INET;
    addr_.sin_addr.s_addr = addr;
    addr_.sin_port = port;
    SYLAR_FMT_DEBUG("create ipv4 addr success, addr: %s:%d", addr, ntohs(port));
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
    SYLAR_FMT_DEBUG("create ipv4 addr success, addr: %s:%d", msg.c_str(), port);
}

const int IPv4Address::get_family() {
    return AF_INET;
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
    char buf[INET_ADDRSTRLEN];
    // try to convert to readable ip info
    if (strcmp(inet_ntop(AF_INET, &addr_.sin_addr.s_addr, buf, INET_ADDRSTRLEN),"") == 0) {
        SYLAR_FMT_ERR("convert ipv4 addr to addr message failed, addr: %ld, err: %s", 
            addr_.sin_addr.s_addr, strerror(errno));
        return "";
    }
    // convert net port to host
    int port = ntohs(addr_.sin_port);
    std::string msg(buf);
    // format "127.0.0.1:8080"
    return msg + ":" + std::to_string(port);
} 

IPv6Address::IPv6Address(const sockaddr_in6& addr): addr6_(addr) {

}

IPv6Address::IPv6Address(uint32_t addr, uint16_t port) {
    if (addr == 0) 
        addr6_.sin6_addr = in6addr_any;
}

IPv6Address::IPv6Address(const std::string& addr, uint16_t port) {
    // convert ipv6 to 
    if (inet_pton(AF_INET6, addr.c_str(), &addr6_.sin6_addr) == -1) {
        SYLAR_FMT_ERR("convert addr message to ipv6 failed, addr: %s, err: %s", 
            addr.c_str(), strerror(errno));
        return;
    }
    // save port 
    addr6_.sin6_family = AF_INET6;
    addr6_.sin6_port = htons(port);
    SYLAR_FMT_DEBUG("create ipv6 addr success, addr: %s:%d", addr.c_str(), port);
}

const int IPv6Address::get_family() {
    return AF_INET6;
}

const sockaddr* IPv6Address::get_sockaddr() {
    return (sockaddr*) (&addr6_);
}

const sockaddr* IPv6Address::get_sockaddr() const {
    return (sockaddr*) (&addr6_);
}

socklen_t IPv6Address::get_sockaddr_len() {
    return sizeof(addr6_);
}

// convert 
const std::string IPv6Address::to_string() {
    // convert addr
    char buf[INET6_ADDRSTRLEN];
    if (strcmp(inet_ntop(AF_INET6, &addr6_.sin6_addr, buf, INET6_ADDRSTRLEN), "") == 0 ) {
        SYLAR_FMT_ERR("convert ipv6 addr to message failed, addr: %ld, err: %s", 
            addr6_.sin6_addr, strerror(errno));
        return "";
    }
    // port 
    int port = ntohs(addr6_.sin6_port);
    return std::string(buf) + ":" + std::to_string(port);
}

}