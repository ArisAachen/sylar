#ifndef __SYLAR_SRC_ADDRESS_H__
#define __SYLAR_SRC_ADDRESS_H__

#include <map>
#include <vector>
#include <memory>
#include <string>

#include <sys/socket.h>
#include <netinet/in.h>


namespace sylar {

class Address {
public:
    typedef std::shared_ptr<Address> ptr;

    /**
     * @brief get local machine info
     * @param[out] ip_vec ip info vec
     * @param[in] family protocol family
     */
    static bool get_ip_info(std::map<std::string, std::vector<Address::ptr>> ip_vec, 
        int family = AF_INET);

    /**
     * @brief parse url to addr
     * @param[out] result parse url result
     * @param[in] host url host
     * @param[in] family socket family
     * @param[in] type socket type
     * @param[in] protocol socket protocol
     */
    static bool look_up(std::vector<Address::ptr>& result, const std::string& host, 
        int family = AF_INET, int type = 0, int protocol = 0);

    /**
     * @brief create address by sockaddr
     * @param[in] addr addr info
     * @param[in] len sock len
     */
    static Address::ptr create(const sockaddr* addr, socklen_t len);

    /**
     * @brief get general sockaddr
     */
    virtual sockaddr* get_sockaddr() = 0;

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
    virtual const int get_family() = 0;

private:
    /// use to storage all type all socket addr 
    sockaddr_storage storage; 
};


class IPv4Address : public Address {
public:
    typedef std::shared_ptr<IPv4Address> ptr;

    /**
     * @brief create ipv4 addr obj
     * @param[in] addr binary type addr
     * @param[in] port addr port 
     */
    IPv4Address(uint32_t addr = htonl(INADDR_ANY), uint16_t port = htons(8080));

    /**
     * @brief create ipv4 addr obj
     * @param[in] addr string type addr
     * @param[in] port addr port 
     */
    IPv4Address(const std::string& addr, uint16_t port);

    /**
     * @brief create ipv4 addr obj
     * @param[in] addr addr info 
     */
    IPv4Address(const sockaddr_in& addr);

    /**
     * @brief get family
     */
    virtual const int get_family() override;

    /**
     * @brief get general sockaddr
     */
    virtual const sockaddr* get_sockaddr() const override;

    /**
     * @brief get general sockaddr
     */
    virtual sockaddr* get_sockaddr() override;

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
     * @brief create ipv6 addr obj
     * @param[in] addr addr info
     */
    IPv6Address(const sockaddr_in6& addr);
    
    /**
     * @brief create ipv4 addr obj
     * @param[in] addr binary type addr
     * @param[in] port addr port 
     */
    IPv6Address(uint32_t addr = 0, uint16_t port = 8080);

    /**
     * @brief create ipv4 addr obj
     * @param[in] addr string type addr
     * @param[in] port addr port 
     */
    IPv6Address(const std::string& addr, uint16_t port);

    /**
     * @brief get addr protocol family
     */
    virtual const int get_family() override;    

    /**
     * @brief get general sockaddr
     */
    virtual sockaddr* get_sockaddr() override;

    /**
     * @brief get general sockaddr
     */
    virtual const sockaddr* get_sockaddr() const override;  

    /**
     * @brief get addr length
     */
    virtual socklen_t get_sockaddr_len() override;      

    /**
     * @brief get readable address message
     */
    virtual const std::string to_string() override;

private:
    /// ipv6 addr 
    sockaddr_in6 addr6_ {};
};

}



#endif