#ifndef __SYLAR_SRC_UTILS_H__
#define __SYLAR_SRC_UTILS_H__

#include "macro.h"

#include <cstdarg>
#include <cstdint>
#include <cstdio>
#include <iostream>
#include <sched.h>
#include <sstream>
#include <string>
#include <fstream>
#include <unistd.h>

namespace sylar {

// StringUtils use to operate string
class StringUtils {
public:
    /**
     * @brief sprintf string
     * @param[in] fmt 
     * @param[in] va args 
     */
    static const std::string sprintf(const char* fmt, ...);
};

// FileUtils use to operate file
class FileUtils  {
public:
    /**
     * @brief save mesage to file
     * @param[in] msg msg
     * @param[in] filepath file path
     */
    static bool save_to_file(const std::string & msg, const std::string & filepath);

    /**
     * @brief load message from file
     * @param[in] msg msg
     * @param[in] filepath file path
     */
    static bool load_from_file(std::string & msg, const std::string & filepath);
};

class SystemInfo {
public:
    /**
     * @brief Get user host name
     */
    static const std::string user();

    /**
     * @brief Get pid
     */
    static uint64_t pid();

    /**
     * @brief Get the elapsed object
     */
    static uint64_t get_elapsed();

    /**
     * @brief Get process name
     */
    static const std::string process_name();
};


}


#endif