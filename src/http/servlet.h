#ifndef __SYLAR_SRC_SERVLET_H__
#define __SYLAR_SRC_SERVLET_H__

#include "http.h"
#include "http_session.h"
#include "../mutex.h"

#include <cstdint>
#include <memory>
#include <string>
#include <functional>
#include <unordered_map>
#include <utility>
#include <vector>

namespace sylar {
namespace http {

class Servlet {
public:
    /// share pointer
    typedef std::shared_ptr<Servlet> ptr;

    /**
     * @brief Construct a new Servlet object
     * @param[in] name name
     */
    Servlet(const std::string& name) : name_(name) {}

    /**
     * @brief Destroy the virtual Servlet object
     */
    virtual~Servlet();

    /**
     * @brief handle request
     * @param[in] req http req
     * @param[in] resp http resp
     * @param[in] session http session
     */
    virtual int32_t handle(HttpRequest::ptr req, HttpResponse::ptr resp, 
        HttpSession::ptr session) = 0;

    /**
     * @brief Get the name object
     */
    const std::string get_name() { return name_; }

protected:
    /// servlet name
    std::string name_ {""};
};

class FunctionServlet : public Servlet {
public:
    /// share pointer
    typedef std::shared_ptr<FunctionServlet> ptr;
    /// callback
    typedef std::function<int32_t (HttpRequest::ptr req, HttpResponse::ptr resp, 
        HttpSession::ptr session)> callback;

    /**
     * @brief Construct a new Function Servlet object
     * @param[in] cb callback
     */
    FunctionServlet(callback cb);

    virtual int32_t handle(HttpRequest::ptr req, HttpResponse::ptr resp, 
        HttpSession::ptr session) override;

private:
    /// callback
    callback cb_ {nullptr};
};

class IServletCreator {
public:
    /// share pointer
    typedef std::shared_ptr<IServletCreator> ptr;

    /**
     * @brief Destroy the virtual I Servlet Creator object
     */
    virtual~IServletCreator();

    /**
     * @brief get servlet
     */
    virtual Servlet::ptr get() const = 0;

    /**
     * @brief Get the name object
     */
    virtual std::string get_name() const = 0;
};

class HoldServletCreator : public IServletCreator {
public:
    /**
     * @brief Construct a new Hold Servlet Creator object
     * @param[in] slt servlet
     */
    HoldServletCreator(Servlet::ptr slt): servlet_(slt) {}

    Servlet::ptr get() const override {
        return servlet_;
    }

    std::string get_name() const override {
        return servlet_->get_name();
    }

private:
    /// servlet
    Servlet::ptr servlet_ {};
};

template<typename T>
class ServletCreator : public IServletCreator {
public:
    /// share pointer
    typedef std::shared_ptr<Servlet> ptr;

    /**
     * @brief Construct a new Servlet Creator object
     */
    ServletCreator() {}

    virtual Servlet::ptr get() const override {
        return Servlet::ptr(new T);
    }

    virtual std::string get_name() const override {
        T t;
        return t.get_name();
    }
};

class ServletDispatch : public Servlet {
public:
    // share pointer
    typedef std::shared_ptr<ServletDispatch> ptr;
    /// mutex
    typedef RWMutex MutexType;

    /**
     * @brief Construct a new Servlet Dispatch object
     * 
     */
    ServletDispatch();

    /**
     * @brief handle http 
     * @param[in] req http request
     * @param[in] resp http response
     * @param[in] session http session
     */
    virtual int32_t handle(HttpRequest::ptr req, HttpResponse::ptr resp, 
        HttpSession::ptr session) override;

    /**
     * @brief add servlet
     * @param[in] uri uri 
     * @param[in] slt servlet
     */
    void add_servlet(const std::string& uri, Servlet::ptr slt);

    /**
     * @brief add servlet 
     * @param[in] uri uri 
     * @param[in] cb callback
     */
    void add_servlet(const std::string& uri, FunctionServlet::callback cb);

    /**
     * @brief add global servlet
     * @param[in] uri uri 
     * @param[in] slt servlet
     */
    void add_global_servlet(const std::string& uri, Servlet::ptr slt);

    /**
     * @brief add servlet 
     * @param[in] uri uri 
     * @param[in] cb callback
     */
    void add_global_servlet(const std::string& uri, FunctionServlet::callback cb);

    /**
     * @brief add servlet creator
     * @param[in] uri uri 
     * @param[in] creator creator
     */
    void add_servlet_creator(const std::string& uri, IServletCreator::ptr creator);

    /**
     * @brief add servlet creator
     * @param[in] uri uri 
     * @param[in] creator creator
     */
    void add_global_servlet_creator(const std::string& uri, IServletCreator::ptr creator);

    template<typename T> 
    void add_servlet_creator(const std::string& uri) {
        add_servlet_creator(uri, ServletCreator<T>::ptr(new T));
    }

    template<typename T>
    void add_global_servlet_creator(const std::string& uri) {
        add_global_servlet_creator(uri, ServletCreator<T>::ptr(new T));
    }

    /**
     * @brief delete servlet
     * @param[in] uri uri
     */
    void del_servlet(const std::string& uri);

    /**
     * @brief delete global servlet
     * @param[in] uri uri
     */
    void del_global_servlet(const std::string& uri);

    /**
     * @brief Get the default object
     */
    Servlet::ptr get_default() { return default_servlet_; }

    /**
     * @brief Set the default object
     * @param[in] slv default servlet
     */
    void set_default(Servlet::ptr slv) { default_servlet_ = slv; }

public:
    /**
     * @brief Get the servlet object
     * @param[in] uri uri 
     */
    Servlet::ptr get_servlet(const std::string& uri);

    /**
     * @brief Get the servlet object
     * @param[in] uri uri 
     */
    Servlet::ptr get_global_servlet(const std::string& uri);

    /**
     * @brief Get the matched servlet object
     * @param[in] uri uri
     */
    Servlet::ptr get_matched_servlet(const std::string& uri);

private:
    /// read write lock
    MutexType mutex_ {};
    /// creator map
    std::unordered_map<std::string, IServletCreator::ptr> creators_ {};
    /// global creator map
    std::vector<std::pair<std::string, IServletCreator::ptr>> global_creators_ {};
    /// default servlet
    Servlet::ptr default_servlet_ {};
};

class NotFoundServlet : public Servlet {
public:
    /// share pointer
    std::shared_ptr<NotFoundServlet> ptr;

    /**
     * @brief Construct a new Not Found Servlet object
     * @param[in] name servlet name
     */
    NotFoundServlet(const std::string& name);

    /**
     * @brief handle http 
     * @param[in] req http request
     * @param[in] resp http response
     * @param[in] session http session
     */
    virtual int32_t handle(HttpRequest::ptr req, HttpResponse::ptr resp, 
        HttpSession::ptr session) override;

private:
    /// servlet name
    std::string name_ {};
    /// content
    std::string content_ {};
};


}
}

#endif