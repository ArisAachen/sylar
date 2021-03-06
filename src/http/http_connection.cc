#include "http.h"
#include "http_parser.h"
#include "http_connection.h"
#include "../log.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <memory>
#include <string>
#include <sstream>
#include <vector>

#include <strings.h>

namespace sylar {
namespace http {


std::string HttpResult::to_string() const {
    std::stringstream ss;
    ss << "[HttpResult result = " << result
    << " error = " << error 
    << " response = " << (response ? response->to_string() : "nullptr")
    << "]";
    return ss.str();
}


HttpConnection::HttpConnection(Socket::ptr sock, bool owner)
    : SocketStream(sock, owner) {
    SYLAR_DEBUG("create http connection");
}

HttpConnection::~HttpConnection() {
    SYLAR_DEBUG("destory http connection");
}

HttpResponse::ptr HttpConnection::recv_response() {
    // response parser
    HttpResponseParser::ptr parser(new HttpResponseParser);
    // create buf
    int size = 4 * 1024;
    std::shared_ptr<char> buffer(new char[size], [](char* p) {
        delete [] p;
    });
    // get data
    char* data = buffer.get();
    int offset = 0;
    do {
        // read socket
        int len = read(data + offset, size - offset);
        if (len < 0) {
            close();
            return nullptr;
        }
        // current offset of data
        // origin offset meams, data offset last time, 
        // add len means current offset
        len += offset;
        // as string
        data[len] = '\0';
        // parse data
        size_t parse_size = parser->execute(data, len);
        // check if error happens
        if (parser->is_error()) {
            close();
            SYLAR_ERR("http response parse failed");
            return nullptr;
        }
        // reset offset to correct size
        offset = len - parse_size;
        // TODO: dont know why check size
        if (offset == size) {
            close();
            return nullptr;
        }
        // parse has finished 
        if (parser->is_finished())
            break;
    } while (true);
    // parse success, should return correct response
    return parser->get_response();
}

int HttpConnection::send_request(HttpRequest::ptr req) {
    // get request
    std::stringstream ss;
    ss << req;
    std::string data = ss.str();
    return write_fix_size(data.c_str(), data.size());
}

HttpResult::ptr HttpConnection::DoGet(const std::string& url, uint64_t timeout, 
        const std::map<std::string, std::string>& header, const std::string& body) {
    // create url
    Uri::ptr uri = Uri::create(url);
    // check if create uri sucessfully
    if (!uri) {
        return HttpResult::ptr(new HttpResult((int)HttpResult::Error::INVALID_URL, 
            nullptr, "invalid url: " + url));
    }
    // try to get
    return DoGet(uri, timeout, header, body);
}

HttpResult::ptr HttpConnection::DoGet(Uri::ptr uri, uint timeout,
        const std::map<std::string, std::string>& headers, const std::string& body) {
    // do request
    return DoRequest(HttpMethod::GET, uri, timeout, headers, body);
}

HttpResult::ptr HttpConnection::DoPost(const std::string& url, uint64_t timeout, 
        const std::map<std::string, std::string>& header, const std::string& body) {
    // create url
    Uri::ptr uri = Uri::create(url);
    // check if create uri sucessfully
    if (!uri) {
        return HttpResult::ptr(new HttpResult((int)HttpResult::Error::INVALID_URL, 
            nullptr, "invalid url: " + url));
    }
    // try to get
    return DoPost(uri, timeout, header, body);
}

HttpResult::ptr HttpConnection::DoPost(Uri::ptr uri, uint timeout,
        const std::map<std::string, std::string>& headers, const std::string& body) {
    // try to get
    return DoRequest(HttpMethod::POST, uri, timeout, headers, body);
}

HttpResult::ptr HttpConnection::DoRequest(HttpMethod method, const std::string& url, uint64_t timeout, 
        const std::map<std::string, std::string>& headers, const std::string& body) {
    // create url
    Uri::ptr uri = Uri::create(url);
    // check if create uri sucessfully
    if (!uri) {
        return HttpResult::ptr(new HttpResult((int)HttpResult::Error::INVALID_URL, 
            nullptr, "invalid url: " + url));
    }
    // try to request
    return DoRequest(method, uri, timeout, headers, body);
}

HttpResult::ptr HttpConnection::DoRequest(HttpMethod method, Uri::ptr uri, uint timeout,
        const std::map<std::string, std::string>& headers, const std::string& body) {
    // create http request
    HttpRequest::ptr req(new HttpRequest);
    req->set_path(uri->get_path());
    req->set_query(uri->get_query());
    req->set_fragment(uri->get_fragment());
    req->set_method(method);
    // add headers
    for (auto& header : headers) {
        // check close tate
        if (strcasecmp(header.first.c_str(), "connection")) 
            if (strcasecmp(header.second.c_str(), "keep-alive"))
                req->set_close(false);
        if (strcasecmp(header.first.c_str(), "host"))
            req->set_header("Host", uri->get_host());
        // add header
        req->set_header(header.first, header.second);
    }
    // set body
    req->set_body(body);
    return DoRequest(req, uri, timeout);
}

HttpResult::ptr HttpConnection::DoRequest(HttpRequest::ptr req, Uri::ptr uri, uint64_t timeout) {
    // create address
    Address::ptr addr = uri->create_address();
    if (!addr)
        return HttpResult::ptr(new HttpResult((int)HttpResult::Error::INVALID_HOST, 
            nullptr, "invalid host: " + uri->get_host()));
    // create socket
    Socket::ptr sock = Socket::create_tcp(addr);
    if (!sock)
        return HttpResult::ptr(new HttpResult((int)HttpResult::Error::CREATE_SOCKET_ERROR, 
            nullptr, "create socket failed, err: " + std::string(strerror(errno))));
    if (!sock->connect(addr))
        return HttpResult::ptr(new HttpResult((int)HttpResult::Error::CONNECT_FAIL, 
            nullptr, "connect failed, err: " + std::string(strerror(errno))));
    HttpConnection::ptr conn(new HttpConnection(sock));
    int rt = conn->send_request(req);
    if (rt == 0) 
        return HttpResult::ptr(new HttpResult((int)HttpResult::Error::SEND_CLOSE_BY_PEER, 
            nullptr, "send request by peer, err: " + std::string(strerror(errno))));
    if (rt < 0)
        return HttpResult::ptr(new HttpResult((int)HttpResult::Error::SEND_SOCKET_ERROR, 
            nullptr, "send request failed, err: " + std::string(strerror(errno))));
    // receive response
    auto resp = conn->recv_response();
    if (!resp) 
        return HttpResult::ptr(new HttpResult((int)HttpResult::Error::TIMEOUT, 
            nullptr, "send request failed, err: " + std::string(strerror(errno))));
    HttpResult::ptr result(new HttpResult((int)HttpResult::Error::OK, resp, "ok"));
    return result;
}

HttpConnectionPool::HttpConnectionPool(const std::string& host, uint32_t port, uint32_t max_size,
    uint32_t max_alive_time, uint32_t max_request) 
    : host_(host)
    , port_(port)
    , max_size_(max_size)
    , max_alive_time_(max_alive_time)
    , max_request_(max_request) {
    
}

HttpConnection::ptr HttpConnectionPool::get_connection() {
    uint64_t ns = SystemInfo::get_elapsed();
    std::vector<HttpConnection::ptr> invalid_conns;
    MutexType::Lock lock(mutex_);
    // remove
    auto pos = std::remove_if(conns_.begin(), conns_.end(), [ns, this](HttpConnection::ptr conn) {
        // check if connection is connected
        if (!conn->is_connected())
            return true;
        // if connection time has expired
        if ((conn->create_time_ + max_alive_time_) > ns) 
            return true;
        return false;
    });
    // remove
    conns_.erase(pos, conns_.end());
    // unlock
    auto conn = conns_.front();
    if (conns_.empty())
        conns_.pop_front();
    lock.unlock();
    // if conn not exist
    if (!conn) {
        // parse hostaddr 
        auto addr = Address::look_up_any(host_);
        if (!addr) {
            SYLAR_FMT_ERR("lookp up addr failed, host: %s", host_.c_str());
            return nullptr;
        }
        // set port
        addr->set_port(port_);
        Socket::ptr sock = Socket::create_tcp(addr);
        if (!sock) {
            SYLAR_FMT_ERR("create addr failed, host: %s", addr->to_string().c_str());
            return nullptr;
        }
        // connect addr
        if (!sock->connect(addr)) {
            SYLAR_FMT_ERR("connect addr failed, host: %s", addr->to_string().c_str());
            return nullptr;
        }
        // create conn
        conn = HttpConnection::ptr(new HttpConnection(sock));
    }
    return conn;
}

HttpResult::ptr HttpConnectionPool::Get(const std::string& url, uint64_t timeout, 
        const std::map<std::string, std::string>& headers, const std::string& body) {
    return Request(HttpMethod::GET, url, timeout, headers, body);
}

HttpResult::ptr HttpConnectionPool::Get(Uri::ptr url, uint64_t timeout, 
        const std::map<std::string, std::string>& headers , const std::string& body) {
    return Request(HttpMethod::GET, url, timeout, headers, body);
}

HttpResult::ptr HttpConnectionPool::Post(const std::string& url, uint64_t timeout, 
        const std::map<std::string, std::string>& headers, const std::string& body) {
    return Request(HttpMethod::POST, url, timeout, headers, body);
}

HttpResult::ptr HttpConnectionPool::Post(Uri::ptr uri, uint64_t timeout, 
        const std::map<std::string, std::string>& headers , const std::string& body) {
    return Request(HttpMethod::POST, uri, timeout, headers, body);
}

HttpResult::ptr HttpConnectionPool::Request(HttpMethod method, const std::string& url, uint64_t timeout, 
        const std::map<std::string, std::string>& headers, const std::string& body) {
    // create uri    
    Uri::ptr uri = Uri::create(url);
    // requesat
    return Request(method, uri, timeout, headers, body);
}

HttpResult::ptr HttpConnectionPool::Request(HttpMethod method, Uri::ptr uri, uint timeout,
        const std::map<std::string, std::string>& headers, const std::string& body) {
    // create http request
    HttpRequest::ptr req(new HttpRequest);
    req->set_path(uri->get_path());
    req->set_query(uri->get_query());
    req->set_fragment(uri->get_fragment());
    req->set_method(method);
    // add headers
    for (auto& header : headers) {
        // check close tate
        if (strcasecmp(header.first.c_str(), "connection")) 
            if (strcasecmp(header.second.c_str(), "keep-alive"))
                req->set_close(false);
        if (strcasecmp(header.first.c_str(), "host"))
            req->set_header("Host", host_);
        // add header
        req->set_header(header.first, header.second);
    }
    // set body
    req->set_body(body);
    return Request(req, uri, timeout);  
}

HttpResult::ptr HttpConnectionPool::Request(HttpRequest::ptr req, Uri::ptr uri, uint64_t timeout) {
    return HttpConnection::DoRequest(req, uri, timeout);
}

}
}