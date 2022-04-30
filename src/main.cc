#include "fiber.h"
#include "scheduler.h"
#include "log.h"
#include "singleton.h"

#include <vector>

void test() {
    SYLAR_INFO(">>>>>> test func");
}

void thread_test() {
    sylar::Scheduler::ptr schedule(new sylar::Scheduler(15, false));
    schedule->start();
}

void fiber_test() {
    std::vector<sylar::Fiber::ptr> vec;
    for (int index = 0; index < 20; index++) {
        vec.emplace_back(sylar::Fiber::ptr(new sylar::Fiber(test, 0, false)));
    }
    sylar::Fiber::get_this();
    for (auto iter : vec) {
        iter->resume();
    }
}

int main () {
    // init log
    sylar::SingletonPtr<sylar::Logger>::get_instance()->init_default();

    SYLAR_INFO("main func start");
    thread_test();
    fiber_test();

    return 1;
}