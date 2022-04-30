#ifndef __SYLAR_SRC_THREAD_H__
#define __SYLAR_SRC_THREAD_H__

#include "log.h"

#include <functional>
#include <memory>
#include <pthread.h>


namespace sylar {
    
class Thread : public std::enable_shared_from_this<Thread> {
public:
    typedef std::shared_ptr<Thread> ptr;
    /**
     * @brief 
     * 
     * @param[in] cb thread func 
     * @param[in] name thread name
     */
    Thread(std::function<void()> cb, const std::string & name);
    /**
     * @brief release thread
     * 
     */
    virtual~Thread();

public:    
    /**
     * @brief run thread
     * 
     */
    void run();
    /**
     * @brief stop thread
     * 
     */
    void stop();

    /**
     * @brief join thread
     */
    void join();

    /**
     * @brief 
     * @param[in] cb thread func
     * @param[in] name thread name 
     */
    void reset(std::function<void(void)>cb, const std::string & name);
    /**
     * @brief get thread id
     */
    pthread_t get_thread_id() { return thread_id_; }

    /**
     * @brief get thread name
     */
    const std::string get_thread_name() { return name_; }

    /**
     * @brief check if thread id is equal
     */
    bool operator==(Thread & thread);

private:
    /**
     * @brief wrap exec func
     * 
     * @param[in] arg thread arg
     */
    static void *wrap(void* arg);

private:
    /// thread exec func
    std::function<void()> cb_ {nullptr};
    /// thread name
    std::string name_ {""};
    /// thread id
    pthread_t thread_id_ {0};
    /// proc id
    pid_t proc_id {0};
};

}

#endif