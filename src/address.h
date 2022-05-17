#ifndef __SYLAR_SRC_ADDRESS_H__
#define __SYLAR_SRC_ADDRESS_H__

#include <memory>

#include <netinet/in.h>
#include <string>
#include <sys/socket.h>

namespace sylar {

class Address {
public:
    typedef std::shared_ptr<Address> ptr;

    /**
     * @brief get general sockaddr
     */
    virtual const sockaddr* get_sockaddr() = 0;

    /**
     * @brief get general sockaddr
     */
    virtual const sockaddr* get_sockaddr() const = 0;

    /**
     * @brief get addr length
     */
    virtual socklen_t get_sockaddr_len() = 0;

    /**
     * @brief get readable address message
     */
    virtual const std::string to_string() = 0;

    /**
     * @brief get addr protocol family
     */
    virtual int get_family();

private:
    /// use to storage all type all socket addr 
    sockaddr_storage storage; 
};


class IPv4Address : public Address {
public:
    typedef std::shared_ptr<IPv4Address> ptr;

    /**
     * @brief create ipv4 address obj
     * @param[in] addr string type addr
     * @param[in] port ipv4 port
     */
    static IPv4Address::ptr create(const std::string& addr, uint16_t port);

    /**
     * @brief create ipv4 addr obj
     * @param[in] addr binary type addr
     * @param[in] port addr port 
     */
    IPv4Address(uint32_t addr = htonl(INADDR_ANY), uint16_t port = 0);

    /**
     * @brief create ipv4 addr obj
     * @param[in] addr string type addr
     * @param[in] port addr port 
     */
    IPv4Address(const std::string& addr, uint16_t port); 

    /**
     * @brief get general sockaddr
     */
    const sockaddr* get_sockaddr() const override;

    /**
     * @brief get general sockaddr
     */
    const sockaddr* get_sockaddr() override;

    /**
     * @brief get addr length
     */
    virtual socklen_t get_sockaddr_len() override;

    /**
     * @brief get readable address message
     */
    virtual const std::string to_string() override;

private:
    /// ipv4 type address
    sockaddr_in addr_ {};
};

class IPv6Address : public  Address {
public:
    typedef std::shared_ptr<IPv6Address> ptr;

    /**
     * @brief create ipv4 address obj
     * @param[in] addr string type addr
     * @param[in] port ipv4 port
     */
    static IPv6Address::ptr create(const std::string& addr, uint16_t port);

    /**
     * @brief create ipv4 addr obj
     * @param[in] addr binary type addr
     * @param[in] port addr port 
     */
    IPv6Address(uint32_t addr = INADDR_ANY, uint16_t port = 0);

    /**
     * @brief create ipv4 addr obj
     * @param[in] addr string type addr
     * @param[in] port addr port 
     */
    IPv6Address(const std::string& addr, uint16_t port); 

private:
    /// ipv6 addr 
    sockaddr_in6 addr6_ {};
};

}



#endif //!__SYLAR_SRC_ADDRESS_H__