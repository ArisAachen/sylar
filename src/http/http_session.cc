#include "http.h"
#include "http_parser.h"
#include "http_session.h"

#include <memory>
#include <string>


namespace sylar {
namespace http {

HttpSession::HttpSession(Socket::ptr sock, bool owner) : 
    SocketStream(sock, owner) {

}

HttpRequest::ptr HttpSession::recv_request() {
    // create parser
    HttpRequestParser::ptr parser(new HttpRequestParser);
    // create buffer
    int size = 4 * 1024;
    std::shared_ptr<char> buffer(new char[size], [](char* p){
        delete [] p;
    });
    char* data = buffer.get();
    int offset = 0;
    do {
        int len = read(data + offset, size - offset);
        if (len < 0) {
            close();
            return nullptr;
        }
        len += offset;
        // parse 
        size_t parse_size = parser->execute(data, len);
        // check if is success
        if (parser->is_error()) {
            close();
            return nullptr;
        }
        offset = len - parse_size;
        if (offset == size) {
            close();
            return nullptr;
        }
        // parser has finished
        if (parser->is_finished())
            break;
    } while (true);
    auto req = parser->get_request();
    req->init();
    return req;
}

int HttpSession::send_response(HttpResponse::ptr resp) {
    std::string msg = resp->to_string();
    return write_fix_size(msg.c_str(), msg.size());
}

}
}





