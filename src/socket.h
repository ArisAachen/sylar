#ifndef __SYLAR_SRC_SOCKET_H__
#define __SYLAR_SRC_SOCKET_H__

#include "address.h"

#include <memory>

#include <sys/socket.h>


namespace sylar {

class Socket : public std::enable_shared_from_this<Socket> {
public:
    typedef std::shared_ptr<Socket> ptr;

public:
    // socket type
    enum Type {
        TCP = SOCK_STREAM,
        UDP = SOCK_DGRAM,
    };
    // socket family
    enum Family {
        IPv4 = AF_INET,
        IPv6 = AF_INET6,
        Unix = AF_LOCAL,
    };

public:
    /**
     * @brief create tcp socket
     * @param[in] addr tcp address info
    */
    static Socket::ptr create_tcp(Address::ptr addr);

    /**
     * @brief create udp socket
     * @param[in] addr udp address info
    */
    static Socket::ptr create_udp(Address::ptr addr);

    /**
     * @brief create tcp socket
    */
    static Socket::ptr create_tcp_socket();

    /**
     * @brief create udp socket
    */
    static Socket::ptr create_udp_socket();

    /**
     * @brief create ipv6 tcp socket
    */
    static Socket::ptr create_tcp_socket6();

    /**
     * @brief create ipv6 udp socket
    */
    static Socket::ptr create_udp_socket6();

    /**
     * @brief create unix tcp socket
    */
    static Socket::ptr create_unix_tcp_socket();

    /**
     * @brief create tcp udp socket
    */
    static Socket::ptr create_unix_udp_socket();

    /**
     * @brief create socket 
     * @param[in] family family
     * @param[in] type socket tye
     * @param[in] protocol socket protocol
     */
    Socket(int family, int type, int protocol = 0);

    /**
     * @brief try to accept 
     */
    Socket::ptr accept();

    /**
     * @brief server bind socket
     * @param[in] addr server addr 
     */
    bool bind(Address::ptr addr);

    /**
     * @brief begin to listen from remote 
     * @param[in] count max listen queue 
     */
    bool listen(int count =  SOMAXCONN);

    /**
     * @brief client try to connect remote
     * @param[in] addr server addr 
     */
    bool connect(Address::ptr addr);

    /**
     * @brief send buffer to tcp type socket
     * @param[in] buf send message buf
     * @param[in] len buf size
     * @param[in] flags send flags
     */
    bool send(void* buf, size_t len, int flags);

    /**
     * @brief send buffer to tcp type socket
     * @param[in] vec send vec
     * @param[in] count vec count
     * @param[in] flags send flags
     */
    bool send(iovec *vec, size_t count, int flags);

    /**
     * @brief send buffer to tcp type socket
     * @param[in] buf send message buf
     * @param[in] len buf size
     * @param[in] addr remote udp addr
     * @param[in] flags send flags
     */
    bool send_to(void* buf, size_t len, Address::ptr addr, int flags);

    /**
     * @brief send buffer to tcp type socket
     * @param[in] buf send message buf
     * @param[in] len buf size
     * @param[in] addr remote udp addr 
     * @param[in] flags send flags
     */
    bool send_to(iovec *vec, int count, Address::ptr addr, int flags);

    /**
     * @brief get local address info 
     */
    Address::ptr get_local_addr() { return local_addr_; }

    /**
     * @brief get remote address info 
     */
    Address::ptr get_remote_addr() { return remote_addr_; }

    /**
     * @brief  get connected state
     */
    bool is_connected() { return connected_; }

    /**
     * @brief destory socket
     */
    virtual~Socket();

private:
    /// file descriptor
    int fd_ {0};
    /// fd family
    int family_ {0};
    /// fd type
    int type_ {0};
    /// fd protocol 
    int protocol_ {0};
    /// connect state
    bool connected_ {false};
    /// local address
    Address::ptr local_addr_ {};
    /// remote address
    Address::ptr remote_addr_ {};
};






}



#endif

