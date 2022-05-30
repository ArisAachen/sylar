#ifndef __SYLAR_SRC_HTTP_SESSION_H__
#define __SYLAR_SRC_HTTP_SESSION_H__


#include "http.h"
#include "../streams/socket_stream.h"
#include <memory>


namespace sylar {
namespace http {


class HttpSession : public SocketStream {
public:
    typedef std::shared_ptr<HttpSession> ptr;

    /**
     * @brief Construct a new Http Session object
     * @param[in] sock socket
     * @param[in] owner owner
     */
    HttpSession(Socket::ptr sock, bool owner = true);

    /**
     * @brief receive request
     */
    HttpRequest::ptr recv_request();

    /**
     * @brief send response
     * @param[in] resp response
     */
    int send_response(HttpResponse::ptr resp);
};


}
}
#endif