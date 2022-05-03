#include "fiber.h"
#include "scheduler.h"
#include "log.h"
#include "singleton.h"

#include <unistd.h>
#include <vector>

void test() {
    SYLAR_DEBUG(">>>>>> execute test func");
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
    sylar::Scheduler::ptr schedule(new sylar::Scheduler(2, false));
    schedule->start();

    for (int index = 0; index < 2; index++) {
        schedule->schedule(test);
    }

    sleep(5);
    SYLAR_INFO("wake up");
    for (int index = 0; index < 2; index++) {
        schedule->schedule(test);
    }
}

void scheduler_test() {
    sylar::Scheduler::ptr schedule(new sylar::Scheduler(2, true));
    for (int index = 0; index < 20; index++) {
        schedule->schedule(test);
    }
    schedule->start();
}


int main () {
    // init log
    sylar::Singleton<sylar::Logger>::get_instance()->init_default();

    // scheduler_thread_test();
    scheduler_test();

    return 1;
}