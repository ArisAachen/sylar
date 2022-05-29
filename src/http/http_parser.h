#ifndef __SYLAR_SRC_HTTP_PARSER_H__
#define __SYLAR_SRC_HTTP_PARSER_H__

#include "http.h"
#include "http-parser/http_parser.h"

#include <cstddef>
#include <memory>
#include <string>
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

    /**
     * @brief Get the parser object
     */
    const http_parser& get_parser() { return parser_; }

    /**
     * @brief Get the request object
     */
    HttpRequest::ptr get_request() { return request_; }

    /**
     * @brief Get the field object
     */
    const std::string get_field() { return field_; }

    /**
     * @brief get finish state
     */
    bool is_finished() { return finished_; } 

    /**
     * @brief is error
     */
    bool is_error() { return !!err_code_; }

    /**
     * @brief Set the error code object
     * @param[in] err_code error code
     */
    void set_error_code(int err_code) { err_code_ = err_code; }

    /**
     * @brief Set the field object
     * @param[in] field field
     */
    void set_field(const std::string& field) { field_ = field; }

    /**
     * @brief Set the finished object
     * @param[in] finished finish state
     */
    void set_finished(bool finished) { finished_ = finished; }

private:
    /// parse error code
    int err_code_ {0};
    /// http request
    HttpRequest::ptr request_ {};
    /// http parser
    http_parser parser_ {};
    /// head field
    std::string field_ {};
    /// finishestate
    bool finished_ {false};
};


class HttpResponseParser {
public:
    typedef std::shared_ptr<HttpResponseParser> ptr;

    /**
     * @brief Construct a new Http Response Parser object
     */
    HttpResponseParser();

    /**
     * @brief parse protocol
     * @param[in] data parse data
     * @param[in] len data len
     */
    size_t execute(char* data, size_t len);

    /**
     * @brief Get the parser object
     */
    const http_parser& get_parser() { return parser_; }

    /**
     * @brief Get the request object
     */
    HttpResponse::ptr get_response() { return response_; }

    /**
     * @brief Get the field object
     */
    const std::string get_field() { return field_; }

    /**
     * @brief is error
     */
    bool is_error() { return !!err_code_; }

    /**
     * @brief finish state
     */
    bool is_finished() { return finished_; }

    /**
     * @brief Set the error code object
     * @param[in] err_code error code
     */
    void set_error_code(int err_code) { err_code_ = err_code; }

    /**
     * @brief Set the field object
     * @param[in] field field
     */
    void set_field(const std::string& field) { field_ = field; }

    /**
     * @brief Set the finished object
     * @param[in] finish finish state
     */
    void set_finished(bool finish) { finished_ = finish; }

private:
    /// parse error code
    int err_code_ {0};
    /// http request
    HttpResponse::ptr response_ {};
    /// http parser
    http_parser parser_ {};
    /// head field
    std::string field_ {};
    /// finish state
    bool finished_ {false};
};


}

}


#endif