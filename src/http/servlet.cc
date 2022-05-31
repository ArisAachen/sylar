#include "servlet.h"
#include "http.h"

#include <cstdint>
#include <utility>
#include <algorithm>

namespace sylar {
namespace http {

FunctionServlet::FunctionServlet(callback cb) 
    : Servlet("FunctionServlet")
    , cb_(cb) {}

int32_t FunctionServlet::handle(HttpRequest::ptr req, HttpResponse::ptr resp, 
    HttpSession::ptr session) {
    return cb_(req, resp, session);
}


ServletDispatch::ServletDispatch()
    : Servlet("ServletDispatch") {
    default_servlet_.reset(new NotFoundServlet("sylar/1.0"));
}

int32_t ServletDispatch::handle(HttpRequest::ptr req, HttpResponse::ptr resp, 
    HttpSession::ptr session) {
    // get servlet
    auto servlet = get_matched_servlet(req->get_path());
    if (servlet) 
        return servlet->handle(req, resp, session);
    return -1;
}

void ServletDispatch::add_servlet(const std::string& uri, Servlet::ptr slt) {
    add_servlet_creator(uri, HoldServletCreator::ptr(new HoldServletCreator(slt)));
}

void ServletDispatch::add_servlet(const std::string& uri, FunctionServlet::callback cb) {
    FunctionServlet::ptr func_servlet(new FunctionServlet(cb));
    add_servlet_creator(uri, HoldServletCreator::ptr(new HoldServletCreator(func_servlet)));
}

void ServletDispatch::add_global_servlet(const std::string& uri, Servlet::ptr slt) {
    MutexType::WriteLock lock(mutex_);
    // try to find the pos
    auto pos = std::find_if(global_creators_.cbegin(), global_creators_.cend(), 
        [&uri](const auto& elem) {
        if (uri == elem.first) 
            return true;
        return false;
    });
    // remove old creator
    if (pos != global_creators_.cend()) 
        global_creators_.erase(pos);
    // append to vec end
    global_creators_.push_back(std::make_pair(uri, HoldServletCreator::ptr(new HoldServletCreator(slt))));
}

void ServletDispatch::add_global_servlet(const std::string& uri, FunctionServlet::callback cb) {
    MutexType::WriteLock lock(mutex_);
    // try to find the pos
    auto pos = std::find_if(global_creators_.cbegin(), global_creators_.cend(), 
        [&uri](const auto& elem) {
        if (uri == elem.first) 
            return true;
        return false;
    });
    // remove old creator
    if (pos != global_creators_.cend()) 
        global_creators_.erase(pos);
    // func servlet
    FunctionServlet::ptr func_servlet(new FunctionServlet(cb));
    global_creators_.push_back(std::make_pair(uri, HoldServletCreator::ptr(new HoldServletCreator(func_servlet))));
}

void ServletDispatch::add_servlet_creator(const std::string& uri, IServletCreator::ptr creator) {
    MutexType::WriteLock lock(mutex_);
    creators_.emplace(uri, creator);
}

void ServletDispatch::add_global_servlet_creator(const std::string& uri, IServletCreator::ptr creator) {
    // push back
    MutexType::WriteLock lock(mutex_);
    global_creators_.push_back(std::make_pair(uri, creator));
}

void ServletDispatch::del_servlet(const std::string &uri) {
    MutexType::WriteLock lock(mutex_);
    creators_.erase(uri);
}

void ServletDispatch::del_global_servlet(const std::string &uri) {
    MutexType::WriteLock lock(mutex_);
    auto pos = std::remove_if(global_creators_.begin(), global_creators_.end(), [&uri](auto& elem) {
        if (elem.first == uri)
            return true;
        return false;
    });
    // remove
    global_creators_.erase(pos, global_creators_.end());
}

Servlet::ptr ServletDispatch::get_servlet(const std::string &uri) {
    MutexType::ReadLock lock(mutex_);
    auto pos = creators_.find(uri);
    // check if not found
    if (pos == creators_.end()) 
        return nullptr;
    return pos->second->get();
}

Servlet::ptr ServletDispatch::get_global_servlet(const std::string &uri) {
    MutexType::ReadLock lock(mutex_);
    auto pos = std::find_if(global_creators_.cbegin(), global_creators_.cend(), [&uri](const auto& elem) {
        if (elem.first == uri) 
            return true;
        return false;
    });
    // check if not found
    if (pos == global_creators_.cend())
        return nullptr;
    return pos->second->get();
}

Servlet::ptr ServletDispatch::get_matched_servlet(const std::string &uri) {
    auto servlet = get_servlet(uri);
    return servlet != nullptr ? servlet : get_global_servlet(uri);
}

NotFoundServlet::NotFoundServlet(const std::string& name) 
    : name_(name)
    , Servlet("NotFoundServlet") {
    content_ = "<html><head><title>404 Not Found"
        "</title></head><body><center><h1>404 Not Found</h1></center>"
        "<hr><center>" + name + "</center></body></html>";
}

int32_t NotFoundServlet::handle(HttpRequest::ptr req, HttpResponse::ptr resp, HttpSession::ptr session) {
    // set response
    resp->set_status(HttpStatus::NOT_FOUND);
    resp->set_header("Server", "sylar/1.0.0");
    resp->set_header("Content-Type", "text/html");
    resp->set_body(content_);
    return 0;
}

}
}