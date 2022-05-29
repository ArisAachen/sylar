#ifndef __SYLAR_SRC_HTTP_H__
#define __SYLAR_SRC_HTTP_H__

#include "http-parser/http_parser.h"

#include <bits/types/time_t.h>
#include <map>
#include <memory>
#include <cstdint>
#include <ostream>
#include <string>


namespace sylar {
namespace http {

// http method
enum class HttpMethod {
#define XX(num, name, string)   name = num,
    HTTP_METHOD_MAP(XX)
#undef XX
    INVALID_METHOD
};

// http status
enum class HttpStatus {
#define XX(code, name, desc) name = code,
    HTTP_STATUS_MAP(XX)
#undef XX
};

/**
 * @brief 忽略大小写比较仿函数
 */
struct CaseInsensitiveLess {
    /**
     * @brief 忽略大小写比较字符串
     */
    bool operator()(const std::string& lhs, const std::string& rhs) const;
};

class HttpResponse;

class HttpRequest {
public:
    // 
    typedef std::shared_ptr<HttpRequest> ptr;

    /// Map struct
    typedef std::map<std::string, std::string, CaseInsensitiveLess> MapType;

    /**
     * @brief Construct a new Http Request object
     * @param[in] version request version
     * @param[in] close if should keep-alive
     */
    HttpRequest(uint8_t version = 0x11, bool close = true);

    /**
     * @brief Create a response object
     */
    std::shared_ptr<HttpResponse> create_response();

public:
    /**
     * @brief Get the method object
     */
    HttpMethod get_method() { return method_; }
    
    /**
     * @brief Get the version object
     */
    uint8_t get_version() { return version_; }

    /**
     * @brief Get the path object
     */
    const std::string get_path() { return path_; }

    /**
     * @brief Get the query object
     */
    const std::string get_query() { return query_; }

    /**
     * @brief Get the body object
     */
    const std::string get_body() { return body_; }

    /**
     * @brief Get the headers object
     */
    const MapType get_headers() { return headers_; }

    /**
     * @brief Get the params object
     */
    const MapType get_params() { return params_; }

    /**
     * @brief Get the cookies object
     */
    const MapType get_cookies() { return cookies_;}

    /**
     * @brief Get the header object
     * @param[in] key header key
     * @param[in] def default value
     */
    const std::string get_header(const std::string& key, const std::string& def = "");

    /**
     * @brief Get the param object
     * @param[in] key param key
     * @param[in] def default value
     */
    const std::string get_param(const std::string& key, const std::string& def = "");

    /**
     * @brief Get the cookie object
     * @param[in] key cookie key
     * @param[in] def default value
     */
    const std::string get_cookie(const std::string& key, const std::string& def = "");

    /**
     * @brief Get the header as object
     * @tparam T type
     * @param[in] key header key
     * @param[in] def default value
     */
    template<typename T>
    T get_header_as(const std::string& key, const T& def = T()) {

    }

    /**
     * @brief Get the param as object
     * @tparam T type
     * @param[in] key param key
     * @param[in] def default value
     */
    template<typename T>
    T get_param_as(const std::string& key, const T& def = T()) {

    }

    /**
     * @brief Get the cookie as object
     * @tparam T type
     * @param[in] key cookie key
     * @param[in] def default value
     */
    template<typename T>
    T get_cookie_as(const std::string& key, const T& def = T()) {
        
    }

    /**
     * @brief if web socket is set
     */
    bool is_websocket() { return websocket_; }

    /**
     * @brief is close is set
     */
    bool is_close() { return close_; }

public:
    /**
     * @brief Set the method object
     * @param[in] method http method
     */
    void set_method(HttpMethod method) { method_ = method; }

    /**
     * @brief Set the version object
     * @param[in] version http version
     */
    void set_version(uint8_t version) { version_ = version; }

    /**
     * @brief Set the path object
     * @param[in] path http path
     */
    void set_path(const std::string& path) { path_ = path; }

    /**
     * @brief Set the query object
     * @param[in] query http query
     */
    void set_query(const std::string& query) { query_ = query; }

    /**
     * @brief Set the body object
     * @param[in] body http body
     */
    void set_body(const std::string& body) { body_ = body; }

    /**
     * @brief Set the fragment object
     * @param[in] fragment http fragment
     */
    void set_fragment(const std::string& fragment) { fragment_ = fragment; }

    /**
     * @brief append the body object
     * @param[in] body http body
     */
    void append_body(const std::string& body) { body_.append(body); }

    /**
     * @brief Set the headers object
     * @param[in] header http header
     */
    void set_headers(const MapType& header) { headers_ = header; }

    /**
     * @brief Set the params object
     * @param[in] params http params
     */
    void set_params(const MapType& params) { params_ = params; }

    /**
     * @brief Set the cookies object
     * @param[in] cookies http cookies
     */
    void set_cookies(const MapType& cookies) { cookies_ = cookies; }

    /**
     * @brief Set the websocket object
     * @param[in] websocket web socket
     */
    void set_websocket(bool websocket) { websocket_ = websocket; }

    /**
     * @brief Set the header object
     * @param[in] key header header
     * @param[in] value header value
     */
    void set_header(const std::string& key, const std::string& value);

    /**
     * @brief Set the param object
     * @param[in] key param header
     * @param[in] value param value
     */
    void set_param(const std::string& key, const std::string& value);

    /**
     * @brief Set the cookie object
     * @param[in] key cookie header
     * @param[in] value cookie value
     */
    void set_cookie(const std::string& key, const std::string& value);

    /**
     * @brief Set the close object
     * @param[in] close keep-alive
     */
    void set_close(bool close) { close_ = close; }

    /**
     * @brief delete header
     * @param[in] key header key
     */
    void del_header(const std::string& key);

    /**
     * @brief delete param
     * @param[in] key param key
     */
    void del_param(const std::string& key);

    /**
     * @brief delete cookie
     * @param[in] key cookie key
     */
    void del_cookie(const std::string& key);

    /**
     * @brief output to stream
     * @param[in] os output strea,
     * @param[in] req http request
     */
    friend std::ostream& operator << (std::ostream& os, HttpRequest& req);

private:
    /**
     * @brief init to parse query and param
     */
    void init_query_param();

    /**
     * @brief init to parse body param
     */
    void init_body_param();

    /**
     * @brief init cookies
     */
    void init_cookies();

    /**
     * @brief init http request
     */
    void init();

private:
    /// http version
    uint8_t version_ {};
    /// http methd
    HttpMethod method_ {HttpMethod::GET};
    /// request path
    std::string path_ {"/"};
    /// request query
    std::string query_ {};
    /// request body
    std::string body_ {};
    /// request fragment
    std::string fragment_ {};
    /// request headers
    MapType headers_ {};
    /// request params
    MapType params_ {};
    /// request cookies
    MapType cookies_ {};
    /// websocket
    bool websocket_ {false};
    /// auto close
    bool close_ {false};
    /// parser flags: 
    /// 0: not parsed, 1: parse url, 2: parse param, 3: prase cookies
    int parser_param_flags_ {0};
};



class HttpResponse {
public:
    /// share ptr
    typedef std::shared_ptr<HttpResponse> ptr;
    /// map type
    typedef std::map<std::string, std::string, CaseInsensitiveLess> MapType;

    /**
     * @brief Construct a new Http Response object
     * @param[in] version 
     * @param[in] close 
     */
    HttpResponse(uint8_t version = 0x11, bool close = true);

    /**
     * @brief Get the status object
     */
    HttpStatus get_status() { return status_; };

    /**
     * @brief Get the version object
     */
    uint8_t get_version() { return version_; }

    /**
     * @brief Get the body object
     */
    const std::string get_body() { return body_; }

    /**
     * @brief Get the reason object
     */
    const std::string get_reason() { return reason_; }

    /**
     * @brief Get the headers object
     */
    const MapType get_headers() { return headers_; }

    /**
     * @brief Get the cookies object
     */
    const MapType get_cookies() { return cookies_;}

    /**
     * @brief Get the header object
     * @param[in] key header key
     * @param[in] def default value
     */
    const std::string get_header(const std::string& key, const std::string& def = "");

    /**
     * @brief Get the cookie object
     * @param[in] key cookie key
     * @param[in] def default value
     */
    const std::string get_cookie(const std::string& key, const std::string& def = "");

    /**
     * @brief Get the header as object
     * @tparam T type
     * @param[in] key header key
     * @param[in] def default value
     */
    template<typename T>
    T get_header_as(const std::string& key, const T& def = T()) {

    }

    /**
     * @brief Get the cookie as object
     * @tparam T type
     * @param[in] key cookie key
     * @param[in] def default value
     */
    template<typename T>
    T get_cookie_as(const std::string& key, const T& def = T()) {
        
    }

    /**
     * @brief if web socket is set
     */
    bool is_websocket() { return websocket_; }

    /**
     * @brief Get the close object
     */
    bool is_close() { return close_; }

public:
    /**
     * @brief Set the status object
     * @param[in] status http status
     */
    void set_status(HttpStatus status) { status_ = status; }

    /**
     * @brief Set the version object
     * @param[in] version http version
     */
    void set_version(uint8_t version) { version_ = version; }

    /**
     * @brief Set the body object
     * @param[in] body http body
     */
    void set_body(const std::string& body) { body_ = body; }

    /**
     * @brief append the body object
     * @param[in] body http body
     */
    void append_body(const std::string& body) { body_.append(body); }

    /**
     * @brief Set the headers object
     * @param[in] header http header
     */
    void set_headers(const MapType& header) { headers_ = header; }

    /**
     * @brief Set the cookies object
     * @param[in] cookies http cookies
     */
    void set_cookies(const MapType& cookies) { cookies_ = cookies; }

    /**
     * @brief Set the websocket object
     * @param[in] websocket web socket
     */
    void set_websocket(bool websocket) { websocket_ = websocket; }

    /**
     * @brief Set the header object
     * @param[in] key header header
     * @param[in] value header value
     */
    void set_header(const std::string& key, const std::string& value);

    /**
     * @brief Set the cookie object
     * @param[in] key cookie header
     * @param[in] value cookie value
     */
    void set_cookie(const std::string& key, const std::string& value, time_t expired = 0, 
        const std::string& path = "", const std::string& domain = "", bool secure = false);

    /**
     * @brief Set the redirect object
     * @param[in] uri uri location
     */
    void set_redirect(const std::string& uri);

    /**
     * @brief Set the close object
     * @param[in] close keep-alive
     */
    void set_close(bool close) { close_ = close; }

    /**
     * @brief delete header
     * @param[in] key header key
     */
    void del_header(const std::string& key);

    /**
     * @brief delete cookie
     * @param[in] key cookie key
     */
    void del_cookie(const std::string& key);

    /**
     * @brief to string
     */
    std::string to_string();
    
    /**
     * @brief output to stream
     * @param[in] os output strea,
     * @param[in] req http request
     */
    friend std::ostream& operator << (std::ostream& os, HttpResponse& req);

private:
    /// http status
    HttpStatus status_ {HttpStatus::OK};
    /// http header 
    MapType headers_ {};
    /// http cookies
    MapType cookies_ {};
    /// http body
    std::string body_ {};
    /// http reason
    std::string reason_ {};
    /// websocket
    bool websocket_ {false};
    /// close 
    bool close_ {false};
    /// version
    uint8_t version_ {};
};


}

}

#endif