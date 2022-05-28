#ifndef __SYLAR_SRC_UTILS_H__
#define __SYLAR_SRC_UTILS_H__

#include "macro.h"

#include <bits/types/time_t.h>
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

    /**
     * @brief decode url info
     * @param[in] url url info
     * @param[in] space_as_plus use space as plus
     */
    static std::string url_decode(const std::string& url, bool space_as_plus = true);

    /**
     * @brief encode url info
     * @param[in] url url info
     * @param[in] space_as_plus use space as plus
     */
    static std::string url_encode(const std::string& url, bool space_as_plus = true);

    /**
     * @brief trim string
     * @param[in] msg trim message
     * @param[in] delimit trim delimit
     */
    static std::string trim(const std::string& msg, const std::string& delimit = " \t\t\n");

    /**
     * @brief time format
     * @param[in] ts time second
     * @param[in] format time format
     */
    static std::string time_format(time_t ts = time(0), const std::string& format = "%Y-%m-%d %H:%M:%S");
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

    /**
     * @brief Get the hook enabled object
     */
    static bool get_hook_enabled();

    /**
     * @brief Set the hook enabled object
     * @param[in] enabled 
     */
    static void set_hook_enabled(bool enabled);
};




}


#endif