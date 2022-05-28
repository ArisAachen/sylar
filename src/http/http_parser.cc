#include "http.h"
#include "http_parser.h"
#include "http-parser/http_parser.h"
#include "../log.h"

#include <cstddef>
#include <cstring>
#include <string>

namespace sylar {
namespace http {

static int on_request_message_begin_cb(http_parser* parser) {
    SYLAR_DEBUG("on_request_message_begin_cb");
    return 0;
}

static int on_request_headers_complete_cb(http_parser* ptr) {
    SYLAR_DEBUG("on_request_headers_complete_cb");
    // convert parser
    HttpRequestParser* parser = static_cast<HttpRequestParser*>(ptr->data);
    // set request version
    parser->get_request()->set_version(((ptr->http_major) << 0x4) | (ptr->http_minor));
    parser->get_request()->set_method((HttpMethod)ptr->method);
    return 0;
}

/**
 * @brief http request parse complete callback
 */
static int on_request_message_complete_cb(http_parser *ptr) {
    SYLAR_DEBUG("on_request_message_complete_cb");
    HttpRequestParser *parser = static_cast<HttpRequestParser *>(ptr->data);
    parser->set_finished(true);
    return 0;
}

/**
 * @brief http request chunk header parse
 */
static int on_request_chunk_header_cb(http_parser *p) {
    SYLAR_DEBUG("on_request_chunk_header_cb");
    return 0;
}

/**
 * @brief http request chunk header parse complete callback
 */
static int on_request_chunk_complete_cb(http_parser *p) {
    SYLAR_DEBUG("on_request_chunk_complete_cb");
    return 0;
}

/**
 * @brief http request url parse complete callback
 */
static int on_request_url_cb(http_parser *p, const char *buf, size_t len) {
    SYLAR_FMT_DEBUG("on_request_chunk_complete_cb, url: %s", std::string(buf, len).c_str());
    // get parser
    HttpRequestParser* parser = static_cast<HttpRequestParser*>(p->data);
    // parse url
    int ret;
    struct http_parser_url url_parser;
    http_parser_url_init(&url_parser);
    ret = http_parser_parse_url(buf, len, 0, &url_parser);
    if (ret != 0) {
        SYLAR_ERR("parse url failed");
        return 1;
    }
    // parse url path
    if (url_parser.field_set & (1 << UF_PATH))
        parser->get_request()->set_path(std::string(buf + url_parser.field_data[UF_PATH].off, 
            url_parser.field_data[UF_PATH].len));
    // parse url query
    if (url_parser.field_set & (1 << UF_QUERY)) 
        parser->get_request()->set_query(std::string(buf + url_parser.field_data[UF_QUERY].off,
            url_parser.field_data[UF_QUERY].len));
    // parse url fragment
    if (url_parser.field_set & (1 << UF_FRAGMENT)) 
        parser->get_request()->set_fragment(std::string(buf + url_parser.field_data[UF_FRAGMENT].off,
            url_parser.field_data[UF_FRAGMENT].len));
    return 0;
}

/**
 * @brief http request header parse complete callback
 */
static int on_request_header_field_cb(http_parser *p, const char *buf, size_t len) {
    std::string field(buf, len);
    SYLAR_FMT_DEBUG("on_request_header_field_cb, field: %s", field.c_str());
    HttpRequestParser* parser = static_cast<HttpRequestParser*>(p->data);
    parser->set_field(field);
    return 0;
}

/**
 * @brief http request header value parse callback
 */
static int on_request_header_value_cb(http_parser *p, const char *buf, size_t len) {
    std::string value(buf, len);
    SYLAR_FMT_DEBUG("on_request_header_value_cb, value: %s", value.c_str());
    HttpRequestParser* parser = static_cast<HttpRequestParser*>(p->data);
    parser->get_request()->set_header(parser->get_field(), value);
    return 0;
}

/**enum http_errno err
 * @brief http request will never call status
 */
static int on_request_status_cb(http_parser *p, const char *buf, size_t len) {
    SYLAR_WARN("on_request_status_cb, should not happen");
    return 0;
}

/**
 * @brief http body chunk callback
 */
static int on_request_body_cb(http_parser *p, const char *buf, size_t len) {
    std::string body(buf, len);
    SYLAR_FMT_DEBUG("on_request_body_cb, body: %s", body.c_str());
    HttpRequestParser* parser = static_cast<HttpRequestParser*>(p->data);
    parser->get_request()->append_body(body);
    return 0;
}

// http request parser setting
static http_parser_settings s_request_settings = {
    .on_message_begin = on_request_message_begin_cb,
    .on_url = on_request_url_cb,
    .on_status = on_request_status_cb,
    .on_header_field = on_request_header_field_cb,
    .on_header_value = on_request_header_value_cb,
    .on_headers_complete = on_request_headers_complete_cb,
    .on_body = on_request_body_cb,
    .on_message_complete = on_request_message_complete_cb,
    .on_chunk_header = on_request_chunk_header_cb,
    .on_chunk_complete = on_request_chunk_complete_cb,
};


HttpRequestParser::HttpRequestParser() {
    // init parser as request parser
    http_parser_init(&parser_, HTTP_REQUEST);
    request_.reset(new HttpRequest);
    parser_.data = this;
    SYLAR_DEBUG("create http request parser");
}

size_t HttpRequestParser::execute(char *data, size_t len) {
    // parse execute
    size_t size = http_parser_execute(&parser_, &s_request_settings, data, len);
    // check if need upgrade, if is need recall execute
    if (parser_.upgrade) {
        SYLAR_DEBUG("http parse found upgrade, ignore");
        set_error_code(HPE_UNKNOWN);
    } else if (parser_.http_errno != 0) {
        // parse failed reasom
        SYLAR_FMT_ERR("http parse failed, err: %s", http_errno_name(HTTP_PARSER_ERRNO(&parser_)));
        set_error_code(parser_.http_errno);
    } else {
        // copy memory
        if (size < len)
            memmove(data, data + size, (len - size));
    }
    // size
    return size;
}

/**
 * @brief http response parse begin callback
 */
static int on_response_message_begin_cb(http_parser* ptr) {
    SYLAR_DEBUG("on_response_message_begin_cb");
    return 0;
}

/**
 * @brief http response headers
 * @note 0: succes   1: no body   2: no body and future
 */
static int on_response_headers_complete_cb(http_parser *p) { 
    SYLAR_DEBUG("on_response_headers_complete_cb");
    HttpResponseParser* parser = static_cast<HttpResponseParser*>(p->data);
    // se version
    parser->get_response()->set_version(((p->http_major) << 0x4) | (p->http_minor));
    parser->get_response()->set_status((HttpStatus)p->status_code);
    return 0;
}

/**
 * @brief http response parse complete callback
 */
static int on_response_message_complete_cb(http_parser *p) {
    SYLAR_DEBUG("on_response_message_complete_cb");
    HttpResponseParser *parser = static_cast<HttpResponseParser *>(p->data);
    parser->set_finished(true);
    return 0;   
}

/**
 * @brief http response chunk header parse callback
 */
static int on_response_chunk_header_cb(http_parser *p) {
    SYLAR_DEBUG("on_response_chunk_header_cb");
    return 0;
}

/**
 * @brief http response chunk complete parse callback
 */
static int on_response_chunk_complete_cb(http_parser *p) {
    SYLAR_DEBUG("on_response_chunk_complete_cb");
    return 0;
}

/**
 * @brief http response never parse url 
 */
static int on_response_url_cb(http_parser *p, const char *buf, size_t len) { 
    SYLAR_DEBUG("on_response_url_cb, should not happen");
    return 0;
}

/**
 * @brief http response parse header field callback
 */
static int on_response_header_field_cb(http_parser *p, const char *buf, size_t len) {
    std::string field(buf, len);
    SYLAR_FMT_DEBUG("on_response_header_field_cb, field: %s", field.c_str());
    HttpResponseParser *parser = static_cast<HttpResponseParser *>(p->data);
    parser->set_field(field);
    return 0;
}

/**
 * @brief http response parse header value callback
 */
static int on_response_header_value_cb(http_parser *p, const char *buf, size_t len) { 
    std::string value(buf, len);
    SYLAR_FMT_DEBUG("on_response_header_value_cb, value: %s", value.c_str());
    HttpResponseParser *parser = static_cast<HttpResponseParser *>(p->data);
    parser->get_response()->set_header(parser->get_field(), value);
    return 0;
}

/**
 * @brief http response parse status callback
 */
static int on_response_status_cb(http_parser *p, const char *buf, size_t len) {
    SYLAR_FMT_DEBUG("on_response_status_cb, status code: %d, status message: %s", p->status_code, std::string(buf, len).c_str());
    HttpResponseParser *parser = static_cast<HttpResponseParser *>(p->data);
    parser->get_response()->set_status((HttpStatus)p->status_code);
    return 0;
}

/**
 * @brief http response parse body callback
 */
static int on_response_body_cb(http_parser *p, const char *buf, size_t len) {
    std::string body(buf, len);
    SYLAR_FMT_DEBUG("on_response_body_cb, body: %s", body.c_str());
    HttpResponseParser *parser = static_cast<HttpResponseParser *>(p->data);
    parser->get_response()->append_body(body);
    return 0;
}

static http_parser_settings s_response_settings = {
    .on_message_begin = on_response_message_begin_cb,
    .on_url = on_response_url_cb,
    .on_status = on_response_url_cb,
    .on_header_field = on_response_header_field_cb,
    .on_header_value = on_response_header_value_cb,
    .on_headers_complete = on_response_headers_complete_cb,
    .on_body = on_response_body_cb,
    .on_message_complete = on_response_message_complete_cb,
    .on_chunk_header = on_response_chunk_header_cb,
    .on_chunk_complete = on_response_chunk_complete_cb,
};


HttpResponseParser::HttpResponseParser() {
    // init parser as http response
    http_parser_init(&parser_, HTTP_RESPONSE);
    response_.reset(new HttpResponse);
    parser_.data = this;
}

size_t HttpResponseParser::execute(char *data, size_t len) {
    size_t size = http_parser_execute(&parser_, &s_response_settings, data, len);
    if (parser_.http_errno != 0) {
        // parse failed reasom
        SYLAR_FMT_ERR("http parse failed, err: %s", http_errno_name(HTTP_PARSER_ERRNO(&parser_)));
        set_error_code(parser_.http_errno);
    } else {
        if (size < len)
            memmove(data, data + size, len - size);
    }
    return size;
}

}
}