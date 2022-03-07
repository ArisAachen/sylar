#include "fiber.h"
#include "log.h"
#include "singleton.h"

#include <vector>

void test() {
    SYLAR_INFO(">>>>>> test func");
}


int main () {
    // init log
    sylar::SingletonPtr<sylar::Logger>::get_instance()->init_default();

    std::vector<sylar::Fiber::ptr> vec;
    for (int index = 0; index < 20; index++) {
        vec.emplace_back(sylar::Fiber::ptr(new sylar::Fiber(test, 0, false)));
    }
    sylar::Fiber::get_this();
    for (auto iter : vec) {
        iter->resume();
    }

    return 1;
}