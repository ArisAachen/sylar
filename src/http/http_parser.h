#ifndef __SYLAR_SRC_HTTP_PARSER_H__
#define __SYLAR_SRC_HTTP_PARSER_H__




#include <cstddef>
#include <memory>
namespace sylar {

namespace http {

class HttpRequestParser {
public:
    typedef std::shared_ptr<HttpRequestParser> ptr;

    /**
     * @brief Construct a new Http Request Parser object
     */
    HttpRequestParser();

    /**
     * @brief parse protocol
     * @param[in] data parse data
     * @param[in] len data len
     */
    size_t execute(char* data, size_t len);

    
};


}

}


#endif