#include "fiber.h"
#include "scheduler.h"
#include "log.h"
#include "singleton.h"

#include <unistd.h>
#include <vector>

void test() {
    SYLAR_INFO(">>>>>> test func");
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

void scheduler_thread_test() {
    SYLAR_INFO(">>>>>> begin scheduler thread test");
    sylar::Scheduler::ptr schedule(new sylar::Scheduler(1, false));
    schedule->start();

    for (int index = 0; index < 1; index++) {
        schedule->schedule(test);
    }

    sleep(5);
    SYLAR_INFO("wake up");
    for (int index = 0; index < 1; index++) {
        schedule->schedule(test);
    }
    SYLAR_INFO("<<<<<< end scheduler thread test");
}

void scheduler_test() {
    SYLAR_INFO("++++++ begin scheduler test ++++++");
    sylar::Scheduler::ptr schedule(new sylar::Scheduler());
    for (int index = 0; index < 20; index++) {
        schedule->schedule(test);
    }
    schedule->start();
    SYLAR_INFO("++++++ end scheduler test ++++++");
}


int main () {
    // init log
    sylar::Singleton<sylar::Logger>::get_instance()->init_default();

    // scheduler_thread_test();
    scheduler_test();

    return 1;
}