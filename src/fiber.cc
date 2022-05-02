#include "fiber.h"
#include "macro.h"
#include "log.h"
#include "scheduler.h"

#include <atomic>
#include <cstddef>
#include <functional>
#include <string>
#include <ucontext.h>


namespace sylar {

class MallocStackAllocator {
public:
static void* Alloc(size_t size) {
    return malloc(size);
}

static void Dealloc(void* vp) {
    return free(vp);
}

};

using StackAllocator = MallocStackAllocator;

static std::atomic<uint64_t> global_fiber_id {0};
static std::atomic<uint64_t> global_fiber_count {0};
// current fiber
static thread_local Fiber::ptr t_fiber = nullptr;
// main fiber
static thread_local Fiber::ptr t_thread_fiber = nullptr;

// 
Fiber::Fiber(std::function<void()>cb, size_t stack_size, bool run_scheduler, const std::string name) : 
    id_(global_fiber_id++), cb_(cb), run_scheduler_(run_scheduler), name_(name) {
    global_fiber_count++;
    stack_size_ = stack_size ? stack_size : 128 * 1024;
    stack_ = StackAllocator::Alloc(stack_size_);
    if (getcontext(&ctx_)) {
        SYLAR_ASSERT(false);
    }

    ctx_.uc_link = nullptr;
    ctx_.uc_stack.ss_sp = stack_;
    ctx_.uc_stack.ss_size = stack_size_;

    // make context
    makecontext(&ctx_, &Fiber::main_func, 0);
    SYLAR_FMT_DEBUG("create child fiber, fiber id: %d", id_);
}

Fiber::Fiber() {
    // set_this(shared_from_this());
    state_ = State::Running;

    if (getcontext(&ctx_)) {
        SYLAR_ASSERT(false);
    }
    ++global_fiber_count;
    id_ = global_fiber_id++;

    SYLAR_FMT_DEBUG("create main fiber, fiber id: %d", id_);
}

Fiber::~Fiber() {
    SYLAR_FMT_INFO("fiber has been destoried, fiber id: %d", id_);
    global_fiber_count--;
    if (name_ == "idle") {
        SYLAR_DEBUG("idle has been called");
    }
    if (stack_) {
        StackAllocator::Dealloc(stack_);
    }
}

void Fiber::reset(std::function<void ()> cb) {
    SYLAR_ASSERT(state_ == State::Term);
    cb_ = cb;
    if (getcontext(&ctx_)) {
        SYLAR_ASSERT(false);
    }
    ctx_.uc_link = nullptr;
    ctx_.uc_stack.ss_sp = stack_;
    ctx_.uc_stack.ss_size = stack_size_;

    state_ = State::Ready;
    makecontext(&ctx_, &Fiber::main_func, 0);
}

void Fiber::main_func() {

    // get curent fiber
    auto fiber = get_this();

    if (fiber->cb_)
        fiber->cb_();
    fiber->cb_ = nullptr;
    fiber->state_ = State::Term;

    // reset fiber
    auto obj = fiber.get();
    fiber.reset();
    obj->yield();
}

void Fiber::set_this(Fiber::ptr obj) {
    t_fiber = obj;
}

Fiber::ptr Fiber::get_this() {
    if (t_fiber)
        return t_fiber;

    // main fiber
    t_thread_fiber = Fiber::ptr(new Fiber());
    t_fiber = t_thread_fiber;
    return t_fiber;
}

void Fiber::resume() {
    set_this(shared_from_this());
    // mark this fiber to running
    state_ = State::Running;
    // if child fiber run in scheduler
    if (run_scheduler_) {
        SYLAR_DEBUG("resume scheduler child fiber");
        swapcontext(&(Scheduler::get_schedule_fiber()->ctx_), &ctx_);
    } else {
        SYLAR_DEBUG("resume thread fiber");
        swapcontext(&t_thread_fiber->ctx_, &ctx_);
    }
}

void Fiber::yield() {
    SYLAR_ASSERT(state_ == State::Running || state_ == State::Term);
    // set main fiber as current fiber
    set_this(t_thread_fiber);
    // mark this fiber to Ready
    state_ = State::Ready;

    if (run_scheduler_) {
        SYLAR_DEBUG("idle scheduler child fiber");
        swapcontext(&ctx_, &(Scheduler::get_schedule_fiber()->ctx_));
    } else {
        SYLAR_DEBUG("idle thread fiber");
        swapcontext(&ctx_, &t_thread_fiber->ctx_);
    }
}


}