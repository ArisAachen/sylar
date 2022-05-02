#ifndef __SYLAR_SRC_FIBER_H__
#define __SYLAR_SRC_FIBER_H__


#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <sys/ucontext.h>
#include <ucontext.h>

namespace sylar {

class Fiber : public std::enable_shared_from_this<Fiber> {
public:
    typedef std::shared_ptr<Fiber> ptr;
    enum class State { Ready, Running, Term };

    /**
     * @brief Construct a new Fiber object
     * @param[in] stack_size 
     * @param[in] run_scheduler 
     */
    Fiber(std::function<void()>cb, size_t stack_size = 0, bool run_scheduler = true, const std::string name = "child");

    /**
     * @brief Destroy the virtual Fiber object
     */
    virtual~Fiber();

    /**
     * @brief 
     * @param[in] cb 
     */
    void reset(std::function<void()> cb);

    /**
     * @brief 
     */
    void resume();

    /**
     * @brief 
     */
    void yield();

    /**
     * @brief Get the state object 
     */
    State get_state() { return state_; }

    /**
     * @brief Get the id object
     */
    uint64_t get_id() { return id_; }

public:
    /**
     * @brief Set the this object
     */
    static void set_this(Fiber::ptr obj);

    /**
     * @brief Get the this object
     */
    static Fiber::ptr get_this();

    /**
     * @brief Get the fiber count object
     */
    static uint64_t get_fiber_count();

    /**
     * @brief Get the fiber id object
     */
    static uint64_t get_fiber_id();

private:
    /**
     * @brief
     */
    static void main_func();

private:
    /**
     * @brief Construct a new Fiber object
     */
    Fiber();

private:
    /// id 
    uint64_t id_ {0};
    /// size
    uint32_t stack_size_ {0};
    /// state
    State state_ {State::Ready};
    /// ctx
    ucontext_t ctx_;
    /// stack
    void* stack_ {nullptr};
    /// function
    std::function<void()> cb_ {nullptr};
    /// run scheduler
    bool run_scheduler_ {false};
    /// fiber name
    std::string name_ {""};
};




}


#endif