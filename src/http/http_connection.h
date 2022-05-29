#ifndef __SYLAR_SRC_HTTP_CONNECTION_H__
#define __SYLAR_SRC_HTTP_CONNECTION_H__

#include "http.h"
#include <memory>
namespace sylar {
namespace http {

/**
 * @brief http response result
 */
struct HttpResult {
    /// share ptr
    typedef std::shared_ptr<HttpResult> ptr;
    
    enum class Error {
        /// success
        OK = 0,
        /// invalid url
        INVALID_URL = 1,
        /// invalid host
        INVALID_HOST = 2,
        /// connect failed
        CONNECT_FAIL = 3,
        /// close by peer
        SEND_CLOSE_BY_PEER = 4,
        /// send socket error
        SEND_SOCKET_ERROR = 5,
        /// time out
        TIMEOUT = 6,
        /// create socket error
        CREATE_SOCKET_ERROR = 7,
        /// get connetion from pool failed
        POOL_GET_CONNECTION = 8,
        /// invalid pool connection
        POOL_INVALID_CONNECTION = 9,
    };

    /**
     * @brief Construct a new Http Result object
     * @param[in] result http result 
     * @param[in] response http response
     * @param[in] error http error
     */
    HttpResult(int result, HttpResponse::ptr response, const std::string& error);

    /**
     * @brief to string
     */
    std::string to_string() const;

public:
    /// http result
    int result;
    /// http response
    HttpResponse::ptr response;
    /// http error
    std::string error;
};


}
}

#endif