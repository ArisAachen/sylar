#ifndef __SYLAR_SRC_HTTP_CONNECTION_H__
#define __SYLAR_SRC_HTTP_CONNECTION_H__

#include "http.h"
#include "../uri.h"
#include "../mutex.h"
#include "../streams/socket_stream.h"

#include <atomic>
#include <list>
#include <cstdint>
#include <memory>
#include <string>
#include <sys/types.h>

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
     * @param[in] rlt http result 
     * @param[in] resp http response
     * @param[in] err http error
     */
    HttpResult(int rlt, HttpResponse::ptr resp, const std::string& err) : 
        result(rlt), response(resp), error(err) {} ;

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


class HttpConnection : public SocketStream {
public:
    /// share poniter
    typedef std::shared_ptr<HttpConnection> ptr;

    /**
     * @brief http get
     * @param[in] url http url 
     * @param[in] timeout http timeout
     * @param[in] header http header
     * @param[in] body http body
     */
    static HttpResult::ptr DoGet(const std::string& url, uint64_t timeout, 
        const std::map<std::string, std::string>& header = {}, const std::string& body = "");

    /**
     * @brief http get
     * @param[in] uri http uri 
     * @param[in] timeout http timeout
     * @param[in] header http header
     * @param[in] body http body
     */
    static HttpResult::ptr DoGet(Uri::ptr uri, uint timeout,
        const std::map<std::string, std::string>& header = {}, const std::string& body = "");

    /**
     * @brief http post
     * @param[in] url http url 
     * @param[in] timeout http timeout
     * @param[in] header http header
     * @param[in] body http body
     */
    static HttpResult::ptr DoPost(const std::string& url, uint64_t timeout, 
        const std::map<std::string, std::string>& header = {}, const std::string& body = "");

    /**
     * @brief http post
     * @param[in] uri http uri 
     * @param[in] timeout http timeout
     * @param[in] header http header
     * @param[in] body http body
     */
    static HttpResult::ptr DoPost(Uri::ptr uri, uint timeout,
        const std::map<std::string, std::string>& header = {}, const std::string& body = "");

    /**
     * @brief http request
     * @param[in] url http url 
     * @param[in] timeout http timeout
     * @param[in] header http header
     * @param[in] body http body
     */
    static HttpResult::ptr DoRequest(HttpMethod method, const std::string& url, uint64_t timeout, 
        const std::map<std::string, std::string>& header = {}, const std::string& body = "");

    /**
     * @brief http request
     * @param[in] uri http uri 
     * @param[in] timeout http timeout
     * @param[in] header http header
     * @param[in] body http body
     */
    static HttpResult::ptr DoRequest(HttpMethod method, Uri::ptr uri, uint timeout,
        const std::map<std::string, std::string>& header = {}, const std::string& body = "");

    /**
     * @brief http request
     * @param[in] req http request
     * @param[in] uri http uri
     * @param[in] timeout http timeout
     */
    static HttpResult::ptr DoRequest(HttpRequest::ptr req, Uri::ptr uri, uint64_t timeout);

    /**
     * @brief Construct a new Http Connection object
     * @param[in] sock socket
     * @param[in] owner owner
     */
    HttpConnection(Socket::ptr sock, bool owner = true);

    /**
     * @brief Destroy the Http Connection object
     */
    ~HttpConnection();

    /**
     * @brief Recv response from http requst
     */
    HttpResponse::ptr recv_response();

    /**
     * @brief Send request
     * @param[in] req send request
     */
    int send_request(HttpRequest::ptr req);

private:
    /// create time
    uint64_t create_time_ {0};
};


class HttpConnectionPool {
public:
    /// share pointer
    typedef std::shared_ptr<HttpConnectionPool> ptr;
    /// mutex type
    typedef Mutex MutexType;

    /**
     * @brief Construct a new Http Connection Pool object
     * @param[in] host http hosat
     * @param[in] port htp port
     * @param[in] max_size pool max size
     * @param[in] max_alive_time max keep alive time
     * @param[in] max_request max request
     */
    HttpConnectionPool(const std::string& host, uint32_t port, uint32_t max_size,
        uint32_t max_alive_time, uint32_t max_request);

    /**
     * @brief Get the connection object
     */
    HttpConnection::ptr get_connection();

    /**
     * @brief http get
     * @param[in] url http url
     * @param[in] timeout http timeout
     * @param[in] headers http headers
     * @param[in] body http body
     */
    HttpResult::ptr Get(const std::string& url, uint64_t timeout, 
        const std::map<std::string, std::string>& headers = {}, const std::string& body = "");
    
    /**
     * @brief http get
     * @param[in] uri http uri
     * @param[in] timeout http timeout
     * @param[in] headers http headers
     * @param[in] body http body
     */
    HttpResult::ptr Get(Uri::ptr uri, uint64_t timeout, 
        const std::map<std::string, std::string>& headers = {}, const std::string& body = "");

    /**
     * @brief http post
     * @param[in] url http url
     * @param[in] timeout http timeout
     * @param[in] headers http headers
     * @param[in] body http body
     */
    HttpResult::ptr Post(const std::string& url, uint64_t timeout, 
        const std::map<std::string, std::string>& headers = {}, const std::string& body = ""); 
    
    /**
     * @brief http post
     * @param[in] uri http uri
     * @param[in] timeout http timeout
     * @param[in] headers http headers
     * @param[in] body http body
     */
    HttpResult::ptr Post(Uri::ptr uri, uint64_t timeout, 
        const std::map<std::string, std::string>& headers = {}, const std::string& body = "");

    /**
     * @brief http request
     * @param[in] url http url 
     * @param[in] timeout http timeout
     * @param[in] header http header
     * @param[in] body http body
     */
    HttpResult::ptr Request(HttpMethod method, const std::string& url, uint64_t timeout, 
        const std::map<std::string, std::string>& header = {}, const std::string& body = "");

    /**
     * @brief http request
     * @param[in] uri http uri 
     * @param[in] timeout http timeout
     * @param[in] header http header
     * @param[in] body http body
     */
    HttpResult::ptr Request(HttpMethod method, Uri::ptr uri, uint timeout,
        const std::map<std::string, std::string>& header = {}, const std::string& body = "");

    /**
     * @brief http request
     * @param[in] req http request
     * @param[in] uri http uri
     * @param[in] timeout http timeout
     */
    HttpResult::ptr Request(HttpRequest::ptr req, Uri::ptr uri, uint64_t timeout);\

private:
    /// url host
    std::string host_ {""};
    /// port
    uint32_t port_ {0};
    /// max size
    uint32_t max_size_ {0};
    /// max alive time
    uint32_t max_alive_time_ {0};
    /// max request size
    uint32_t max_request_ {0};
    /// mutex
    MutexType mutex_ {};
    /// conn list
    std::list<HttpConnection::ptr> conns_ {};
    /// avaliable connection num
    std::atomic<int32_t> total_ {};
};



}
}

#endif