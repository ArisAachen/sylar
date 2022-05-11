#include "utils.h"
#include <bits/types/FILE.h>
#include <bits/types/struct_timespec.h>
#include <cstdint>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <ios>
#include <string>
#include <unistd.h>

namespace sylar {

const std::string StringUtils::sprintf(const char* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    char* buf = nullptr;
    size_t size = vasprintf(&buf, fmt, ap);
    SYLAR_ASSERT(size < 0);
    va_end(ap);
    std::string ret(buf, size);
    free(buf);
    return ret;    
}

// save to file
bool FileUtils::save_to_file(const std::string &msg, const std::string &filepath) {
    std::ofstream file(filepath, std::ios_base::out | std::ios_base::binary);
    // check if open successfully
    if (!file.is_open())
        return false;
    file << msg;
    file.close();
    return true;
}

// load from file
bool FileUtils::load_from_file(std::string &msg, const std::string &filepath) {
    std::ifstream file(filepath, std::ios_base::in | std::ios_base::binary);
    // check if open successfully
    if (!file.is_open())
        return false;
    file >> msg;
    file.close();
    return true;
}

// get user host name
const std::string SystemInfo::user() {
        static std::string user;
        if (user == "") {
            std::ifstream file("/etc/hostname");
            // check if open file successfully
            if (!file.is_open()) {
                return "";
            }
            file >> user;
            // close file
            file.close();
        }
        return user;    
}

// get millionseconds
uint64_t SystemInfo::get_elapsed() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    return ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
}

// get pid 
uint64_t SystemInfo::pid() {
    return getpid();
}

const std::string SystemInfo::process_name() {
    return "proc";
}

}