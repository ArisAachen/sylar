#ifndef __SYLAR_SRC_URI_H__
#define __SYLAR_SRC_URI_H__


#include "address.h"
#include <cstdint>
#include <memory>
#include <ostream>
#include <string>

namespace sylar {

/*
    foo://user@sylar.com:8042/over/there?name=ferret#nose
    \_/   \______________/\_________/ \_________/ \__/
    |           |            |            |        |
    scheme     authority       path        query   fragment    
*/ 

class Uri {
public:
    typedef std::shared_ptr<Uri> ptr;

    /**
     * @brief Create a uri object
     * @param[in] uri uri path
     */
    static Uri::ptr create(const std::string& uri);

    /**
     * @brief Construct a new Uri object
     */
    Uri();

    /**
     * @brief Get the scheme object
     */
    const std::string get_scheme() const { return scheme_; }

    /**
     * @brief Get the user info object
     */
    const std::string get_user_info() const { return user_info_; }

    /**
     * @brief Get the host object
     */
    const std::string get_host() const { return host_; }

    /**
     * @brief Get the path object
     */
    const std::string get_path() const { return path_.empty() ? "/" : path_; }

    /**
     * @brief Get the query object
     */
    const std::string get_query() const { return query_; }

    /**
     * @brief Get the fragement object
     */
    const std::string get_fragment() const { return fragment_; }

    /**
     * @brief Get the port object
     */
    int32_t get_port() const;

public:
    /**
     * @brief Set the scheme object
     * @param[in] scheme uri scheme
     */
    void set_scheme(const std::string& scheme) { scheme_ = scheme; }

    /**
     * @brief Set the user info object
     * @param[in] userinfo uri user info
     */
    void set_user_info(const std::string& userinfo) { user_info_ = userinfo; }

    /**
     * @brief Set the host object
     * @param[in] host uri host
     */
    void set_host(const std::string& host) { host_ = host; }

    /**
     * @brief Set the path object
     * @param[in] path uri path
     */
    void set_path(const std::string& path) { path_ = path; }

    /**
     * @brief Set the query object
     * @param[in] query uri query
     */
    void set_query(const std::string& query) { query_ = query; }

    /**
     * @brief Set the fragment object
     * @param[in] fragment uri fragment
     */
    void set_fragment(const std::string& fragment) { fragment_ = fragment; }

    /**
     * @brief Set the port object
     * @param[in] port uri port
     */
    void set_port(int32_t port) { port_ = port; }

public:
    /**
     * @brief Create a address object
     */
    Address::ptr create_address();

    /**
     * @brief parse uri to string
     */
    std::string to_string();

    /**
     * @brief overwrite uri to stream
     * @param[in] os output stream
     * @param[in] uri uri path
     */
    friend std::ostream& operator << (std::ostream& os, Uri& uri);

private:
    /// uri scheme
    std::string scheme_ {};
    /// user info 
    std::string user_info_ {};
    /// host
    std::string host_ {};
    /// uri path
    std::string path_ {};
    /// query
    std::string query_ {};
    /// fragment
    std::string fragment_ {};
    /// port
    int32_t port_ {0};
};

}

#endif