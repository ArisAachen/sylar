#include "http.h"
#include "../log.h"
#include "../utils.h"

#include <cstdint>
#include <cstring>
#include <ostream>
#include <string>
#include <strings.h>

namespace sylar {
namespace http {
    
#define PARSE_PARAM(str, m, flag, trim)                                                                    \
    size_t pos = 0;                                                                                        \
    do {                                                                                                   \
        size_t last = pos;                                                                                 \
        pos         = str.find('=', pos);                                                                  \
        if (pos == std::string::npos) {                                                                    \
            break;                                                                                         \
        }                                                                                                  \
        size_t key = pos;                                                                                  \
        pos        = str.find(flag, pos);                                                                  \
                                                                                                           \
        if (0) {                                                                                           \
            std::cout << "<key>:" << str.substr(last, key - last)                                          \
                      << " <decoded>:" << StringUtils::url_decode(str.substr(last, key - last))       \
                      << " <value>:" << str.substr(key + 1, pos - key - 1)                                 \
                      << " <decoded>:" <<StringUtils::url_decode(str.substr(key + 1, pos - key - 1)) \
                      << std::endl;                                                                        \
        }                                                                                                  \
                                                                                                           \
        m.insert(std::make_pair(StringUtils::url_decode(trim(str.substr(last, key - last))),          \
                                StringUtils::url_decode(str.substr(key + 1, pos - key - 1))));        \
        if (pos == std::string::npos) {                                                                    \
            break;                                                                                         \
        }                                                                                                  \
        ++pos;                                                                                             \
    } while (true);

// string
static const char* s_method_string[] = {
#define XX(num, name, string) #string,
    HTTP_METHOD_MAP(XX)
#undef XX
};

static const char* http_method_to_string(HttpMethod method) {
    uint32_t idx = (uint32_t)method;
    if (idx > (sizeof(s_method_string) / sizeof(s_method_string[0]))) 
        return "<unknown>";
    return s_method_string[idx];
}

static const char* http_status_to_string(HttpStatus status) {
    switch (status) {
#define XX(code, name, msg) \
    case HttpStatus::name:  \
        return #msg;
    HTTP_STATUS_MAP(XX)
#undef XX
    default:
        return "<unknown>";
    }
}

bool CaseInsensitiveLess::operator()(const std::string &lhs, const std::string &rhs) const {
    return strcasecmp(lhs.c_str(), rhs.c_str()) < 0;
}

HttpRequest::HttpRequest(uint8_t version, bool close)
    : version_(version)
    , close_(close) {
    SYLAR_DEBUG("create http request");
}

const std::string HttpRequest::get_header(const std::string &key, const std::string& def) {
    auto it = headers_.find(key);
    return it == headers_.end() ? def : it->second;
}

const std::string HttpRequest::get_param(const std::string &key, const std::string& def) {
    // TODO: should know how this works
    init_query_param();
    init_body_param();
    auto it = params_.find(key);
    return it == params_.end() ? def : it->second;
}

const std::string HttpRequest::get_cookie(const std::string &key, const std::string& def) {
    init_cookies();
    auto it = cookies_.find(key);
    return it == cookies_.end() ? def : it->second;
}

void HttpRequest::set_header(const std::string &key, const std::string &value) {
    headers_.insert(key, value);
}

void HttpRequest::set_param(const std::string &key, const std::string &value) {
    params_.insert(key, value);
}

void HttpRequest::set_cookie(const std::string &key, const std::string &value) {
    cookies_.insert(key, value);
}

void HttpRequest::del_header(const std::string &key) {
    headers_.erase(key);
}

void HttpRequest::del_param(const std::string &key) {
    params_.erase(key);
}

void HttpRequest::del_cookie(const std::string &key) {
    params_.erase(key);
}

std::ostream& operator << (std::ostream& os, HttpRequest& req) {
    // GET /uri HTTP/1.1
    // Host: wwww.sylar.top
    os << http_method_to_string(req.method_) << " "
    << req.path_ 
    << (req.query_.empty() ? "" : "?") 
    << "HTTP/" 
    << (uint32_t)(req.version_ >> 4) << 
    "." 
    << (uint32_t)(req.version_ & 0x0F)
    << "\r\n";

    // connection: "keep-alive"
    if (req.websocket_) 
        os << "connection: " << (req.close_ ? "close" : "keep-alive") << "\r\n";

    // headers
    for (auto& it : req.headers_) {
        // check if is websockett
        if (!req.websocket_ && strcasecmp(it.first.c_str(), "connection") == 0) 
            continue;
        // check if body is empty
        if (!req.body_.empty() && strcasecmp(it.first.c_str(), "content-length") == 0) 
            continue;
        os << it.first << ": " << it.second << "\r\n";
    }

    // add body
    if (!req.body_.empty())
        os << "content-length: " << req.body_.size() << "\r\n\r\n" 
            << req.body_;
    else 
        os << "\r\n";

    return os;
}

void HttpRequest::init_query_param() {
    // check if already parsed
    if (parser_param_flags_ & 0x1) 
        return;

    PARSE_PARAM(query_, params_, '&', );
    parser_param_flags_ |= 0x1;
    SYLAR_FMT_DEBUG("http request query init, body: %s, param: %s", query_.c_str());
}

void HttpRequest::init_body_param() {
    if (parser_param_flags_ & 0x2) 
        return;
    // get content type
    std::string content_type = get_header("content-type");
    // check 
    if (strcasestr(content_type.c_str(), "application/x-www-form-urlencoded") == nullptr) {
        parser_param_flags_ |= 0x2;
        return;
    }
    PARSE_PARAM(body_, params_, '&', );
    parser_param_flags_ |= 0x2;
    SYLAR_FMT_DEBUG("http request body init, body: %s, param: %s", body_.c_str());
}

void HttpRequest::init_cookies() {
    if (parser_param_flags_ & 0x4)
        return;
    // get cookies
    std::string cookie = get_header("cookie");
    if (cookie.empty()) {
        parser_param_flags_ |= 0x4;
        return;
    }
    PARSE_PARAM(cookie, cookies_, ';', StringUtils::trim);
    parser_param_flags_ |= 0x4;
    SYLAR_FMT_DEBUG("http request cookie init, body: %s, param: %s", cookie.c_str());
}

void HttpRequest::init() {
    std::string conn = get_header("connection");
    if (!conn.empty()) {
        if (strcasecmp(conn.c_str(), "keep-alive") == 0) 
            close_ = false;
        else 
            close_ = true;
    }
}

HttpResponse::HttpResponse(uint8_t version, bool close)
    : version_(version), close_(close) {
    SYLAR_DEBUG("create http response");
}

const std::string HttpResponse::get_header(const std::string &key, const std::string& def) {
    auto it = headers_.find(key);
    return it == headers_.end() ? def : it->second;
}

const std::string HttpResponse::get_cookie(const std::string &key, const std::string& def) {
    auto it = cookies_.find(key);
    return it == cookies_.end() ? def : it->second;
}

void HttpResponse::set_header(const std::string& key, const std::string& value) {
    headers_.insert(key, value);
}

void HttpResponse::set_redirect(const std::string& uri) {
    status_ = HttpStatus::FOUND;
    set_header("Location", uri);
}

void HttpResponse::set_cookie(const std::string& key, const std::string& value, time_t expired, 
        const std::string& path, const std::string& domain, bool secure) {
    std::string result(value);
    if (expired > 0) {
        result.append(";expires=");
        result.append(StringUtils::time_format(expired, "%a, %d %b %Y %H:%M:%S"));
        result.append(" GMT");
    }
    if (!domain.empty())
        result.append(";domain=").append(domain);
    if (!path.empty())
        result.append(";path=").append(path);
    if (secure)
        result.append(";secure");
    SYLAR_FMT_DEBUG("http response set cookie: %s", result.c_str());
    cookies_.insert(key, result);
}

void HttpResponse::del_header(const std::string &key) {
    headers_.erase(key);
}

void HttpResponse::del_cookie(const std::string &key) {
    headers_.erase(key);
}

std::ostream& operator << (std::ostream& os, HttpResponse& resp) {
    // HTTP/1.1 304 Not Modified
    // Date: Thu, 07 Jun 2012 07:21:36 GMT
    // Connection: close
    os << "HTTP/"
    << (uint32_t)(resp.version_ >> 4)
    << "."
    << (uint32_t)(resp.version_ & 0x0F)
    << " "
    << (uint32_t)resp.status_
    << " "
    << (resp.reason_.empty() ? http_status_to_string(resp.status_) : resp.reason_)
    << "\r\n";
    // add header
    for (auto& it : resp.headers_) {
        // add connection
        if (!resp.websocket_ && strcasecmp(it.first.c_str(), "connection") == 0)
            continue;
        os << it.first << ": " << it.second << "\r\n";
    }
    // append cookie
    for (auto& it : resp.cookies_) {
        os << "Set-Cookie: " << it.first << "=" << it.second << "\r\n";
    }
    // add close
    if (!resp.websocket_)
        os << "connection: " << (resp.close_ ? "close" : "keep-alive") << "\r\n";

    if (!resp.body_.empty()) 
        os << "content-length: " << resp.body_.size() << "\r\n\r\n" 
            << resp.body_;
    else 
        os << "\r\n";

    return os;
}

}
}