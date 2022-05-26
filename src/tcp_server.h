#ifndef __SYLAR_SRC_TCP_SERVER_H__
#define __SYLAR_SRC_TCP_SERVER_H__

#include "socket.h"
#include "address.h"
#include "iomanager.h"
#include "noncopyable.h"

#include <memory>
#include <string>
#include <vector>


namespace sylar {


class TcpServer : Noncopyable, 
            public std::enable_shared_from_this<TcpServer> {
public:
    typedef std::shared_ptr<TcpServer> ptr;

    /**
     * @brief Construct a new Tcp Server object
     * @param[in] io_worker 
     * @param[in] accept_worker socket accept scheduler
     */
    TcpServer(const std::string& name, int type, IOManager::ptr io_worker = IOManager::get_scheduler(), 
        IOManager::ptr accept_worker = IOManager::get_scheduler());

    /**
     * @brief Destroy the virtual Tcp Server object
     */
    virtual~TcpServer();

    /**
     * @brief tcp server bind
     * @param[in] addr server addr
     */
    virtual bool bind(Address::ptr addr);

    /**
     * @brief bind addr vec
     * @param[in] addrs bind addr vec
     * @param[out] fails addr fail vec
     */
    virtual bool bind(const std::vector<Address::ptr> addrs, std::vector<Address::ptr> fails);

    /**
     * @brief start server
     */
    virtual bool start();

    /**
     * @brief stop server
     */
    virtual bool stop();

protected:
    /**
     * @brief handle connect socket
     * @param[in] client client socket
     */
    virtual void handle_client(Socket::ptr client);

    /**
     * @brief handle accept socket
     * @param[in] sock server accept
     */
    virtual void start_accept(Socket::ptr sock);

private:
    /// socket vec
    std::vector<Socket::ptr> sockets_;
    /// io worker
    IOManager::ptr io_worker_ {nullptr};
    /// accept workder
    IOManager::ptr accept_worker_ {nullptr};
    /// server name
    std::string name_ {""};
    /// server type
    int type_ {0};
    /// running state
    bool running_ {false};
};


}

#endif