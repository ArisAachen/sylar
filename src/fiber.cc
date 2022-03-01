#include "fiber.h"
#include "macro.h"
#include "log.h"

#include <atomic>
#include <cstddef>
#include <cstdlib>
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
static thread_local Fiber::ptr t_fiber = nullptr;
static thread_local Fiber::ptr t_thread_fiber = nullptr;

// 
Fiber::Fiber(std::function<void()>cb, size_t stack_size, bool run_scheduler) : 
    id_(global_fiber_id++), cb_(cb), run_scheduler_(run_scheduler) {
    global_fiber_count++;
    stack_size_ = stack_size ? stack_size : 128 * 1024;
    stack_ = StackAllocator::Alloc(stack_size_);
    if (getcontext(&ctx_)) {
        SYLAR_ASSERT(false);
    }

    ctx_.uc_link = nullptr;
    ctx_.uc_stack.ss_sp = stack_;
    ctx_.uc_stack.ss_size = stack_size_;


}   

Fiber::~Fiber() {
    SYLAR_FMT_INFO("fiber has been destoried, fiber id: %d", id_);
    global_fiber_count--;
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
}

void Fiber::resume() {
    set_this(Fiber::ptr(this));

    
}


}