#ifndef __SYLAR_SRC_HTTP_SERVER_H__
#define __SYLAR_SRC_HTTP_SERVER_H__

#include "../tcp_server.h"
#include "servlet.h"
#include <memory>
#include <string>

namespace sylar {
namespace http {

class HttpServer : public TcpServer {
public:
    /// share pointer
    typedef std::shared_ptr<HttpServer> ptr;

    /**
     * @brief Construct a new Http Server object
     * @param[in] keepalive alive state
     * @param[in] worker worker manager
     * @param[in] io_worker io worker manager
     * @param[in] accept_worker accept worker manager
     */
    HttpServer(bool keepalive = false, IOManager::ptr worker = IOManager::get_scheduler(), 
        IOManager::ptr io_worker= IOManager::get_scheduler(), IOManager::ptr accept_worker = IOManager::get_scheduler());

    /**
     * @brief Get the servlet dispatch object 
     */
    ServletDispatch::ptr get_servlet_dispatch() { return dispatch_; }
    
    /**
     * @brief Set the servlet dispatch object
     * @param dispatch 
     */
    void set_servlet_dispatch(ServletDispatch::ptr dispatch);

    /**
     * @brief Set the name object
     * @param[in] name name
     */
    void set_name(const std::string& name) override;

protected:
    /**
     * @brief handle connect socket
     * @param[in] client client socket
     */
    virtual void handle_client(Socket::ptr client) override;    

private:
    /// if support alive
    bool keep_alive {false};
    /// servlet dispatch
    ServletDispatch::ptr dispatch_ {};
};





}
}

#endif