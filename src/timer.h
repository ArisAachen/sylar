#ifndef __SYLAR_SRC_TIMER_H__
#define __SYLAR_SRC_TIMER_H__

#include "mutex.h"

#include <cstdint>
#include <set>
#include <memory>
#include <string>
#include <functional>


namespace sylar {

// pre define here
class TimerManager;

// timer
class Timer : public std::enable_shared_from_this<Timer> {
public:
    friend class TimerManager;
    typedef std::shared_ptr<Timer> ptr;

    /**
     * @brief refresh this timer
     */
    void refresh();

    /**
     * @brief cancel this timer
     */
    void cancel();

    /**
     * @brief reset this timer to time
     * @param inter time seconds
     */
    void reset(uint64_t inter);

private:
    /**
     * @brief Construct a new Timer object
     * @param inter timeout time
     * @param recurring if this timer is recurring
     * @param cb timeout callback
     */
    Timer(uint64_t inter, bool recurring, std::function<void()> cb, std::string name = "");

private:
    /**
     * @brief compare timer to set
     */
    struct Comparator {
        bool operator() (const Timer::ptr first, const Timer::ptr second) const {
            if (first->interval_ > second->interval_) 
                return false;
            return true;
        }
    };

private:
    /// time recurring
    bool recurring_ {false};
    /// execute time interval
    uint64_t interval_ {0};
    /// timeout callback
    std::function<void()> cb_ {nullptr};
    /// timer name
    std::string name_ {""};
};


// timer manager
class TimerManager : std::enable_shared_from_this<TimerManager> {
public: 
    typedef std::shared_ptr<TimerManager> ptr;
    typedef RWMutex MutexType;

    /**
     * @brief Construct a new Timer Manager object
     */
    TimerManager();

    /**
     * @brief Destroy the Timer Manager object
     */
    ~TimerManager();

    /**
     * @brief add timer to this manager
     * @param[in] ms time 
     * @param[in] recurring if time need recurring 
     * @param[in] cb callback
     * @param[in] name timer name
     */
    void add_timer(uint64_t ms, bool recurring, std::function<void()> cb, std::string name);

    /**
     * @brief if current timer include elem
     */
    bool empty();

private:
    /// rwlock 
    MutexType mutex_ {};
    /// timer set
    std::multiset<Timer::ptr, Timer::Comparator> timers_;
    /// last timeout time
    uint64_t prev_time_ {0};
};



}

#endif