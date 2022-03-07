#include "log.h"

#include <array>
#include <cstdlib>
#include <locale>
#include <ctime>
#include <iomanip>
#include <memory>
#include <mutex>
#include <sstream>
#include <string>
#include <tuple>
#include <utility>
#include <vector>
#include <map>
#include <iostream>
#include <functional>

#include <sys/syslog.h>

namespace sylar {

// level_to_string use to convert class type level to string
const std::string LogLevel::level_to_string(Level level) {
    switch (level) {
// type to string
#define TTS(name)  \
    case Level::name: \
        return #name;
    // return string level
    TTS(Debug);
    TTS(Info);
    TTS(Warn);
    TTS(Err);
    TTS(Fatal);
    default:
        break;
    }
    // if all level is not match, level is default info 
    return "Info";
}

// string_to_level use to convert string to class type
const LogLevel::Level LogLevel::string_to_level(const std::string &level) {
// string to type
#define STT(name) \
    if (level == #name) \
        return Level::name;
    // return class level
    STT(Debug);
    STT(Info);
    STT(Warn);
    STT(Err);
    STT(Fatal);
    // if level is not match, level is default info
    return Level::Info;
}

LogFormater::LogFormater(const std::string pattern):
log_pattern_(pattern) {
    items_.clear();
}

LogFormater::~LogFormater() {
    items_.clear();
}


// StringFormatItem use to format simple message
class StringFormatItem : public LogFormater::FormaterItem {
public:
    // use base constructor
    using LogFormater::FormaterItem::FormaterItem;

    // override base virtual format method
    virtual std::ostream & format(std::ostream & os, LogLevel::Level level, LogEvent::ptr event) override {
        return os << extend_;
    }
};

// DateTimeFormatItem %d{%Y-%m-%d %H:%M:%S} 
class DateTimeFormatItem : public LogFormater::FormaterItem {
public:
    // use base constructor
    using LogFormater::FormaterItem::FormaterItem;

    // override base virtual format method
    virtual std::ostream & format(std::ostream & os, LogLevel::Level level, LogEvent::ptr event) override {
        // get time clock
        auto seconds = std::chrono::system_clock::to_time_t(event->get_clock());
        return os << std::put_time(std::localtime(&seconds), extend_.c_str());
    }
};

// TableFormatItem %T
class TableFormatItem : public LogFormater::FormaterItem {
public:
    // use base constructor
    using LogFormater::FormaterItem::FormaterItem;

    // override base virtual format method
    virtual std::ostream & format(std::ostream & os, LogLevel::Level level, LogEvent::ptr event) override {
        return os << "\t";
    }
};

// UserFormatItem %u
class UserFormatItem : public LogFormater::FormaterItem {
public:
    // use base constructor
    using LogFormater::FormaterItem::FormaterItem;

    // override base virtual format method
    virtual std::ostream & format(std::ostream & os, LogLevel::Level level, LogEvent::ptr event) override {
        return os << event->get_user();
    }
};

// ProcNameFormatItem %N
class ProcNameFormatItem : public LogFormater::FormaterItem {
public:
    // use base constructor
    using LogFormater::FormaterItem::FormaterItem;

    // override base virtual format method
    virtual std::ostream & format(std::ostream & os, LogLevel::Level level, LogEvent::ptr event) override {
        return os << event->get_name();
    }
};

// ProcIdFormatItem %p
class ProcIdFormatItem : public LogFormater::FormaterItem {
public:
    // use base constructor
    using LogFormater::FormaterItem::FormaterItem;

    // override base virtual format method
    virtual std::ostream & format(std::ostream & os, LogLevel::Level level, LogEvent::ptr event) override {
        return os << event->get_pid();
    }
};

// LogLevelFormatItem %L
class LogLevelFormatItem : public LogFormater::FormaterItem {
public:
    // use base constructor
    using LogFormater::FormaterItem::FormaterItem;

    // override base virtual format method
    virtual std::ostream & format(std::ostream & os, LogLevel::Level level, LogEvent::ptr event) override {
        return os << LogLevel::level_to_string(level);
    }
};

// FineNameFormatItem %f
class FineNameFormatItem : public LogFormater::FormaterItem {
public:
    // use base constructor
    using LogFormater::FormaterItem::FormaterItem;

    // override base virtual format method
    virtual std::ostream & format(std::ostream & os, LogLevel::Level level, LogEvent::ptr event) override {
        return os << event->get_file();
    }
}; 

// FuncNameFormatItem %c
class FuncNameFormatItem : public LogFormater::FormaterItem {
public:
    // use base constructor
    using LogFormater::FormaterItem::FormaterItem;

    // override base virtual format method
    virtual std::ostream & format(std::ostream & os, LogLevel::Level level, LogEvent::ptr event) override {
        return os << event->get_func();
    }
}; 

// FuncLineFormatItem %l
class FuncLineFormatItem : public LogFormater::FormaterItem {
public:
    // use base constructor
    using LogFormater::FormaterItem::FormaterItem;

    // override base virtual format method
    virtual std::ostream & format(std::ostream & os, LogLevel::Level level, LogEvent::ptr event) override {
        return os << event->get_line();
    }
};

// FuncLineFormatItem %l
class MessageFormatItem : public LogFormater::FormaterItem {
public:
    // use base constructor
    using LogFormater::FormaterItem::FormaterItem;

    // override base virtual format method
    virtual std::ostream & format(std::ostream & os, LogLevel::Level level, LogEvent::ptr event) override {
        return os << event->get_message();
    }
};

// LineFormatItem %n
class NewLineFormatItem : public LogFormater::FormaterItem {
public:
    // use base constructor
    using LogFormater::FormaterItem::FormaterItem;

    // override base virtual format method
    virtual std::ostream & format(std::ostream & os, LogLevel::Level level, LogEvent::ptr event) override {
        return os << "\n";
    }
};

// init format to vec
void LogFormater::init() {
    // clear old 
    // items_.clear();
    // type extend success
    std::vector<std::tuple<std::string, std::string, bool>> vec;
    // parse format
    for (int index = 0; index < log_pattern_.size(); index++) {
        // should ignore % character
        if (log_pattern_[index] == '%')
            continue;
        
        // parse time format here
        // %d{%Y-%m-%d %H:%M:%S}
        if (log_pattern_[index] == 'd') {
            int n = index + 1;
            // check if time format is legal
            if (log_pattern_[n] != '{') {
                vec.emplace_back("d", "%Y-%m-%d %H:%M:%S", true);
                break;
            }
            // search time format end
            while (n < log_pattern_.size()) {
                // add 
                if (log_pattern_[n] != '}') {
                    n++;
                    continue;
                }
                // add date time item 
                std::string date = log_pattern_.substr(index+2, n-index-2);
                vec.emplace_back("d", log_pattern_.substr(index+2, n-index-2), true);
                index = n + 1;
                break;
            }
        }
        // add other item
        vec.emplace_back(std::string(1, log_pattern_[index]), "", true);
    }

    // format items
    static const std::map<std::string, std::function<FormaterItem::ptr(const std::string &)>> items = {
#define MAKE_MAP(str, C) \
        {#str, [](const std::string & name) { return FormaterItem::ptr(new C(name)); }}
        
        MAKE_MAP(d, DateTimeFormatItem), 
        MAKE_MAP(T, TableFormatItem),
        MAKE_MAP(u, UserFormatItem),
        MAKE_MAP(N, ProcNameFormatItem),
        MAKE_MAP(p, ProcIdFormatItem),
        MAKE_MAP(L, LogLevelFormatItem),
        MAKE_MAP(f, FineNameFormatItem),
        MAKE_MAP(c, FuncNameFormatItem),
        MAKE_MAP(l, FuncLineFormatItem),
        MAKE_MAP(m, MessageFormatItem),
        MAKE_MAP(n, NewLineFormatItem)
    };
    // search format item 
    for (auto iter : vec) {
        auto finder = items.find(std::get<0>(iter));
        // cannot find format, output origin style
        if (finder == items.end()) {
            items_.emplace_back(FormaterItem::ptr(new StringFormatItem(std::get<0>(iter))));
        } else {
            // append 
            items_.emplace_back(finder->second(std::get<1>(iter)));
        }
    }
}

// format to string
const std::string LogFormater::format(LogLevel::Level level, LogEvent::ptr event) {
    std::stringstream ss;
    format(ss, level, event);
    return ss.str();
}

// format to os
std::ostream & LogFormater::format(std::ostream & os, LogLevel::Level level, LogEvent::ptr event) {
    for (auto iter : items_)
        os << iter->format(level, event);
    return os;
}

// StdoutLogAppender print log to appender
class StdoutLogAppender : public LogAppender {
public:
    // use default appender
    StdoutLogAppender(const std::string & pattern = "") {
        formater_.reset(new LogFormater);
    }

    // init
    virtual void init() override {
        formater_->init();
    }

    /**
     * @brief Set the level object
     * @param[in] level set log level 
     */
    virtual void set_level(LogLevel::Level level) override {
        level_ = level;
    }

    /**
     * @brief 
     * @param[in] level log level
     * @param[in] event
     */
    virtual void log(LogLevel::Level level, LogEvent::ptr event) override {
        // should lock here
        std::lock_guard<std::mutex> lock(mutex_);
        // if current level has lower prioperty
        // should ignore this level
        if (level > level_) 
            return;
        // log to cout
        formater_->format(std::cout, level, event);
    }
};

// 
class SysLogAppender : public LogAppender {
public:
    /**
     * @brief Construct a new Sys Log Appender object
     */
    SysLogAppender(const std::string & pattern = "%T%f:%c:%l%T%m") {
        formater_.reset(new LogFormater(pattern));
    }

    virtual void init() override {
        formater_->init();
        // open sys log
        openlog("", LOG_CONS | LOG_PID | LOG_NDELAY , LOG_USER);
        // use debug as default
        setlogmask(LOG_UPTO(level_to_syslog(level_)));
    }

    /**
     * @brief Destroy the virtual Sys Log Appender object
     */
    virtual~SysLogAppender() {
        // close sys log here
        closelog();
    }

    /**
     * @brief convert level to sys log level
     * @param level log level
     * @details sys log define
     * % LOG_EMERG	    0
     * % LOG_ALERT      1
     * % LOG_CRIT       2
     * % LOG_ERR        3
     * % LOG_WARNING    4
     * % LOG_NOTICE	    5
     * % LOG_INFO	    6
     * % LOG_DEBUG	    7
     */
    static int level_to_syslog(LogLevel::Level level) {
        // define system log
        // use alert as fatal here
        enum Syslog { Emerg, Fatal, Crit, Err, Warn, Notice, Info, Debug};
#define TOSYS(index) \
        if (LogLevel::Level::index == level) \
            return index;
        TOSYS(Fatal);
        TOSYS(Err);
        TOSYS(Warn);
        TOSYS(Info);
        TOSYS(Debug);
        // use info as default level
        return Info;
    }

    /**
     * @brief Set the level object
     * @param[in] level set log level 
     */
    virtual void set_level(LogLevel::Level level) override {
        // set log
        level_ = level;
        // set sys log mask
        setlogmask(LOG_UPTO(level_to_syslog(level)));
    }

    /**
     * @brief 
     * @param[in] level log level
     * @param[in] event
     */
    virtual void log(LogLevel::Level level, LogEvent::ptr event) override {
        // should lock here
        std::lock_guard<std::mutex> lock(mutex_);
        // if current level has lower prioperty
        // should ignore this level
        if (level > level_) 
            return;
        // log to cout
        std::stringstream ss;
        formater_->format(ss, level, event);
        // log to sys log
        syslog(level_to_syslog(level), ss.str().c_str(), "");
    }
};

Logger::Logger() {
    appenders_.clear();
}

Logger::~Logger() {
    appenders_.clear();
}

// init default
void Logger::init_default() {
    // add system log
    auto system_log = SysLogAppender::ptr(new SysLogAppender());
    system_log->init();
    add_appender("syslog", system_log);

    // add console log
    auto console_log = StdoutLogAppender::ptr(new StdoutLogAppender());
    console_log->init();
    add_appender("console", console_log);
}

// add log appender
void Logger::add_appender(const std::string &name, LogAppender::ptr appender) {
    // try to find appender
    auto iter = appenders_.find(name);
    // already exist, ignore
    if (iter != appenders_.end())
        return;
    // insert
    appenders_.insert(std::make_pair(name, appender));
}

// delete appender
void Logger::delete_appender(const std::string &name) {
    if (name == "") 
        return;
    appenders_.erase(name);
}

// log
void Logger::log(LogLevel::Level level, LogEvent::ptr event) {
    for (auto iter : appenders_) 
        iter.second->log(level, event);
}

// debug
void Logger::debug(LogEvent::ptr event) {
    log(LogLevel::Level::Debug, event);
}

// info
void Logger::info(LogEvent::ptr event) {
    log(LogLevel::Level::Info, event);
}

// warn
void Logger::warn(LogEvent::ptr event) {
    log(LogLevel::Level::Warn, event);
}

// err
void Logger::error(LogEvent::ptr event) {
    log(LogLevel::Level::Err, event);
}

// err
void Logger::fatal(LogEvent::ptr event) {
    log(LogLevel::Level::Fatal, event);
    // should exist
    exit(-1);
}



}