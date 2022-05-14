#ifndef __SYLAR_SRC_FDMANAGER_H__
#define __SYLAR_SRC_FDMANAGER_H__

#include "mutex.h"
#include "singleton.h"
#include <cstdint>
#include <memory>
#include <vector>

namespace sylar {

class FdCtx : public std::enable_shared_from_this<FdCtx> {
public:
    typedef std::shared_ptr<FdCtx> ptr;

    /**
     * @brief Construct a new Fd Ctx object
     * @param[in] fd file descriptor
     */
    FdCtx(int fd);

    /**
     * @brief Destroy the virtual Fd Ctx object
     */
    virtual~FdCtx();

    /**
     * @brief Set the nonblock object
     * @param[in] block nonblock state
     */
    void set_nonblock(bool nonblock);

    /**
     * @brief Set the timeout object
     * @param[in] timeout timeout
     */
    void set_timeout(int timeout);

    /**
     * @brief Get the fd object
     */
    int get_fd() { return fd_; }

    /**
     * @brief if current fd is socket type
     */
    bool is_socket() { return is_socket_; }

    /**
     * @brief if current fd is nonblock
     */
    bool is_nonblock() { return is_nonblock_; }

    /**
     * @brief Get the timeout object
     */
    int get_timeout() { return timeout_; }

private:
    /**
     * @brief init fd context
     */
    void init();

private:
    /// file descriptor
    int fd_ {0};
    /// if fd is socket
    bool is_socket_ {false};
    /// if id is nonblock
    bool is_nonblock_ {false};
    /// timeout
    int timeout_ {-1};
};


class FdManager : public std::enable_shared_from_this<FdManager> {
public:
    typedef std::shared_ptr<FdManager> ptr;
    typedef RWMutex MutexType;

    /**
     * @brief Construct a new Fd Manager object
     */
    FdManager();

    /**
     * @brief Destroy the Fd Manager object
     */
    ~FdManager();

    /**
     * @brief add fd to vec
     * @param[in] fd add fd
     */
    void add_fdctx(int fd);

    /**
     * @brief delete fd from vector
     * @param[in] fd del fd
     */
    void del_fdctx(int fd);

    /**
     * @brief Get the fdctx object
     * @param[in] fd fd 
     */
    FdCtx::ptr get_fdctx(int fd);

private:
    /// read write lock
    MutexType mutex_;
    /// fd context vector
    std::vector<FdCtx::ptr> fd_vec_;
};

typedef Singleton<FdManager> FdMgr;

}

#endif