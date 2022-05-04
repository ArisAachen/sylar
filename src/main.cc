#include "fiber.h"
#include "iomanager.h"
#include "scheduler.h"
#include "log.h"
#include "singleton.h"

#include <vector>
#include <cstring>

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

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
    sylar::Scheduler::ptr schedule(new sylar::Scheduler(1, true));
    for (int index = 0; index < 20; index++) {
        schedule->schedule(test);
    }
    schedule->start();
}

void io_manager_test() {
    sylar::IOManager::ptr manager(new sylar::IOManager(1, true, "IO Manager"));
    // create socket
    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in server_addr;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(12344);
    server_addr.sin_family = AF_INET;
    // bind socket
    if(bind(listen_fd, (sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        SYLAR_FMT_ERR("bind listen socket failed, err: %s", strerror(errno));
        return;
    }
    // listen socket 
    listen(listen_fd, 10);
    auto listen_callback = [&]() {
        // accept from listen fd
        SYLAR_DEBUG("listen callback is called");
        int apt_fd = accept(listen_fd, nullptr, nullptr);
        SYLAR_FMT_DEBUG("accept from listen fd successfully, fd: %d", apt_fd);
        // read and write
        auto read_callback = [&]() {
            SYLAR_DEBUG("read callback is called");
            char buf[512];
            read(apt_fd, buf, 512);
            SYLAR_FMT_DEBUG(">>>>>> receive message from remote, message: %s", buf);
        };
        manager->add_fd_event(apt_fd, sylar::IOManager::Event::READ, read_callback);
    };
    manager->add_fd_event(listen_fd, sylar::IOManager::Event::READ, listen_callback);
    manager->start();
}


int main () {
    // init log
    sylar::Singleton<sylar::Logger>::get_instance()->init_default();

    // scheduler_thread_test();
    // scheduler_test();

    // SYLAR_INFO("++++++ io test start");
    io_manager_test();
    // SYLAR_INFO("++++++ io test end");

    return 1;
}