#ifndef __SYLAR_SRC_STREAM_H__
#define __SYLAR_SRC_STREAM_H__

#include "bytearray.h"

#include <cstddef>
#include <memory>


namespace sylar {

class Stream {
public:
    typedef std::shared_ptr<Stream> ptr;

    /**
     * @brief Destroy the virtual Stream object
     */
    virtual~Stream() {}

    /**
     * @brief read buf from stream 
     * @param[out] buf buf pointer
     * @param[in] len buf len
     */
    virtual int read(void* buf, size_t len) = 0;

    /**
     * @brief read buf to byte array
     * @param[out] arr byte array
     * @param[in] len buf len
     */
    virtual int read(ByteArray::ptr arr, size_t len) = 0;

    /**
     * @brief read fix size to buf
     * @param[out] buf buf pointer
     * @param[in] len buf len
     */
    virtual int read_fix_size(void* buf, size_t len);

    /**
     * @brief read fix size to buf
     * @param[out] arr byte array
     * @param[in] len buf len
     */
    virtual int read_fix_size(ByteArray::ptr arr, size_t len);

    /**
     * @brief write buf to stream
     * @param[in] buf buf pointer
     * @param[in] len buf len
     */
    virtual int write(const void* buf, size_t len) = 0;

    /**
     * @brief write byte array to stream
     * @param[in] arr byte array
     * @param[in] len buf len
     */
    virtual int write(ByteArray::ptr arr, size_t len) = 0;

    /**
     * @brief write buf to stream
     * @param[in] buf buf pointer
     * @param[in] len buf len
     */    
    virtual int write_fix_size(const void* buf, size_t len);

    /**
     * @brief write byte array to stream
     * @param[in] arr byte array
     * @param[in] len buf len
     */ 
    virtual int write_fix_size(ByteArray::ptr arr, size_t len);

    /**
     * @brief close stream
     */
    virtual void close();
};

}

#endif