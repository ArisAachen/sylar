#ifndef __SYLAR_SRC_SOCKET_H__
#define __SYLAR_SRC_SOCKET_H__

#include "address.h"

#include <cstddef>
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
     * @brief Destroy the virtual Socket object
     */
    virtual~Socket();

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
    int send(const void* buf, size_t len, int flags = 0);

    /**
     * @brief send buffer to tcp type socket
     * @param[in] vec send vec
     * @param[in] count vec count
     * @param[in] flags send flags
     */
    int send(const iovec *vec, size_t count, int flags = 0);

    /**
     * @brief send buffer to tcp type socket
     * @param[in] buf send message buf
     * @param[in] len buf size
     * @param[in] addr remote udp addr
     * @param[in] flags send flags
     */
    int send_to(const void* buf, size_t len, Address::ptr addr, int flags = 0);

    /**
     * @brief send buffer to tcp type socket
     * @param[in] buf send message buf
     * @param[in] count buf count
     * @param[in] addr remote udp addr 
     * @param[in] flags send flags
     */
    int send_to(const iovec *vec, int count, Address::ptr addr, int flags = 0);

    /**
     * @brief receive message from tcp type socket
     * @param[in] buf receive message buf
     * @param[in] len message size
     * @param[in] flags receive flags
     */
    int recv(void* buf, size_t len, int flags = 0);

    /**
     * @brief receive message from tcp type socket
     * @param[in] buf receive message buf
     * @param[in] count buf count
     * @param[in] flags receive flags
     */
    int recv(iovec* buf, size_t count, int flags = 0);

    /**
     * @brief receive message from udp type socket
     * @param[in] buf receive message buf
     * @param[in] len message size
     * @param[in] addr remote udp addr 
     * @param[in] flags receive flags
     */
    int recv_from(void *buf, size_t len, Address::ptr addr, int flags = 0);

    /**
     * @brief receive message from udp type socket
     * @param[in] buf receive message buf
     * @param[in] len buf count
     * @param[in] addr remote udp addr 
     * @param[in] flags receive flags
     */
    int recv_from(iovec *buf, size_t count, Address::ptr addr, int flags = 0);

    /**
     * @brief cancel read from fd, if read is overwrite by hook
     */
    void cancel_read();

    /**
     * @brief cancel write from fd, if read is overwrite by hook
     */
    void cancel_write();

    /**
     * @brief cancel all from fd, if read is overwrite by hook
     */
    void cancel_all();

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
     * @brief close socket
     */
    bool close();

public:
    /**
     * @brief Get the fd object
     */
    int get_fd() { return fd_; }

    /**
     * @brief Get the family object
     */
    int get_family() { return family_; }

    /**
     * @brief Get the type object
     */
    int get_type() { return type_; }

    /**
     * @brief Get the protocol object
     */
    int get_protocol() { return protocol_; }

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

