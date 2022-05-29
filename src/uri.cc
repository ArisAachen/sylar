#include "uri.h"
#include "address.h"
#include "http/http-parser/http_parser.h"
#include "log.h"
#include "http/http_parser.h"
#include <cstdint>
#include <ostream>
#include <sstream>
#include <string>



namespace sylar {


Uri::ptr Uri::create(const std::string &url_info) {
    // create uri obj
    Uri::ptr uri(new Uri);
    // init url parser
    struct http_parser_url parser;
    http_parser_url_init(&parser);
    /// begin parse
    if (http_parser_parse_url(url_info.c_str(), url_info.length(), 0, &parser) != 0) {
        SYLAR_ERR("create uri failed");
        return nullptr;
    }
    // check if include schema
    if (parser.field_set & (1 << UF_SCHEMA)) {
        std::string scheme(url_info.c_str() + parser.field_data[UF_SCHEMA].off, 
            parser.field_data[UF_SCHEMA].len);
        uri->set_scheme(scheme);
    }
    // check if include user info
    if (parser.field_set & (1 << UF_USERINFO)) {
        std::string userinfo(url_info.c_str() + parser.field_data[UF_USERINFO].off, 
            parser.field_data[UF_USERINFO].len);
        uri->set_user_info(userinfo);
    }
    // check if include host
    if (parser.field_set & (1 << UF_HOST)) {
        std::string host(url_info.c_str() + parser.field_data[UF_HOST].off, 
            parser.field_data[UF_HOST].len);
        uri->set_host(host);
    }
    // check if include port
    if (parser.field_set & (1 << UF_PORT)) {
        std::string port_str(url_info.c_str() + parser.field_data[UF_PORT].off, 
            parser.field_data[UF_PORT].len);
        int32_t port = std::stoi(port_str);
        uri->set_port(port);
    } else {
        // default http port
        if (uri->get_scheme() == "http" || uri->get_scheme() == "ws")
            uri->set_port(80);
        else if (uri->get_scheme() == "https")
            uri->set_port(443);
    }
    // check if include path
    if (parser.field_set & (1 << UF_PATH)) {
        std::string path(url_info.c_str() + parser.field_data[UF_PATH].off, 
            parser.field_data[UF_PATH].len);
        uri->set_path(path);
    }
    // check if include query
    if (parser.field_set & (1 << UF_QUERY)) {
        std::string query(url_info.c_str() + parser.field_data[UF_QUERY].off, 
            parser.field_data[UF_QUERY].len);
        uri->set_query(query);
    }
    // check if include fragement
    if (parser.field_set & (1 << UF_FRAGMENT)) {
        std::string fragement(url_info.c_str() + parser.field_data[UF_FRAGMENT].off, 
            parser.field_data[UF_FRAGMENT].len);
        uri->set_fragment(fragement);
    }
    return uri;
}

Uri::Uri() : port_(0) {}

int32_t Uri::get_port() const {
    if (port_) 
        return port_;
    if (scheme_ == "http" || scheme_ == "ws")
        return 80;
    if (scheme_ == "https") 
        return 443;
    return port_;
}

std::string Uri::to_string() {
    std::stringstream ss;
    ss << scheme_
    << "://" 
    << user_info_ 
    << (user_info_.empty() ? "" : "@")
    << host_ << ":" << get_port()
    << path_
    << (query_.empty() ? "" : "?")
    << query_
    << (fragment_.empty() ? "" : "#")
    << fragment_;
    return ss.str();
}

std::ostream& operator << (std::ostream& os, Uri& uri) {
    return os << uri.to_string();
}

Address::ptr Uri::create_address() {
    auto addr = Address::look_up_any(host_);
    if (addr)
        addr->set_port(port_);
    return addr;
}

}