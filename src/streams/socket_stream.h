#ifndef __SYLAR_SRC_SOCKET_STREAM_H__
#define __SYLAR_SRC_SOCKET_STREAM_H__

#include "../socket.h"
#include "../stream.h"


#include <cstddef>
#include <memory>


namespace sylar {

class SocketStream : public Stream {
public:
    /// share pointer
    typedef std::shared_ptr<SocketStream> ptr;

    /**
     * @brief Construct a new Socket Stream object
     * @param[in] sock socket 
     * @param[in] owner if totally control
     */
    SocketStream(Socket::ptr sock, bool owner = true);

    /**
     * @brief Destroy the Socket Stream object
     */
    ~SocketStream();

    /**
     * @brief read from buf
     * @param[out] buf read buf
     * @param[in] length read len
     */
    virtual int read(void* buf, size_t length) override;

    /**
     * @brief read from buf
     * @param[out] arr read byte array
     * @param[in] length read len
     */
    virtual int read(ByteArray::ptr arr, size_t length) override;

    /**
     * @brief write buf to socket
     * @param[in] buf write buf
     * @param[in] length write len
     */
    virtual int write(const void* buf, size_t length) override;

    /**
     * @brief write to buf
     * @param[in] arr write byte array
     * @param[in] length write length
     */
    virtual int write(const ByteArray::ptr arr, size_t length) override;

    /**
     * @brief close stream
     */
    virtual void close() override;

    /**
     * @brief connected state
     */
    bool is_connected() { return sock_->is_connected(); }

    /**
     * @brief Get the remote addr object
     */
    Address::ptr get_remote_addr();

    /**
     * @brief Get the local addr object
     */
    Address::ptr get_local_addr();

private:
    /// socket
    Socket::ptr sock_ {};
    /// owner
    bool owner_ {false};
};

}

#endif