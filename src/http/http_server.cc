#include "http_server.h"
#include "http.h"
#include "http_session.h"
#include "servlet.h"
#include "../log.h"

#include <memory>

namespace sylar {
namespace http {

HttpServer::HttpServer(bool keepalive, IOManager::ptr worker, IOManager::ptr io_worker, 
    IOManager::ptr accept_worker) 
    : TcpServer(io_worker, accept_worker)
    , keep_alive(keepalive) {
    dispatch_.reset(new ServletDispatch);
}

void HttpServer::set_name(const std::string &name) {
    TcpServer::set_name(name);
    dispatch_->set_default(std::make_shared<NotFoundServlet>(name));
}

void HttpServer::handle_client(Socket::ptr client) {
    // create session
    HttpSession::ptr session(new HttpSession(client));
    do {
        // recv request from socket
        auto req = session->recv_request();
        if (!req) {
            SYLAR_ERR("cant recv request");
            break;
        }
        HttpResponse::ptr resp(new HttpResponse(req->get_version(), req->is_close() || !keep_alive));
        resp->set_header("Server", get_name());
        dispatch_->handle(req, resp, session);
        session->send_response(resp);
        if (!keep_alive || req->is_close()) 
            break;
    } while (true);
    // close session
    session->close();
}

}
}