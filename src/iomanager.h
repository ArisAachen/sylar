#ifndef __SYLAR_SRC_IOMANAGER_H__
#define __SYLAR_SRC_IOMANAGER_H__

#include "fiber.h"
#include "scheduler.h"

#include <map>
#include <list>
#include <memory>
#include <functional>
#include <sys/epoll.h>

namespace sylar {


class IOManager : public Scheduler {
public:
    typedef std::shared_ptr<IOManager> ptr;

    /**
     * @brief Construct a new IOManager object
     * @param threads 
     * @param use_caller 
     * @param name 
     */
    IOManager(size_t threads = 1, bool use_caller = true, const std::string& name = "Scheduler");

    /**
     * @brief Destroy the IOManager object
     */
    ~IOManager();
    
    /**
     * @brief 
     * @param fd 
     * @param events 
     */
    void del_fd_event(int fd, EPOLL_EVENTS events);

protected:
    /**
     * @brief idle to add new tasks from epoll wait
     * @details 
     */
    virtual void idle() override;

public:
    /**
     * @brief epoll event
     */
    enum class Event {
        /// none event
        NONE = 0x0,
        /// read EPOLLIN
        READ = 0x1,
        /// write EPOLLOUT
        WRITE = 0x4,
        // write and read
        RW = READ | WRITE,
    };

private:
    /**
     * @brief tranform epoll event to event
     * @param events epoll events
     */
    static Event epoll_to_event(uint32_t events);

    /**
     * @brief tranform event to epoll event
     * @param events 
     */
    static uint32_t event_to_epoll(Event events);

public:
    /**
     * @brief use to add 
     * @param fd 
     * @param events 
     */
    void add_fd_event(int fd, Event events, std::function<void()>cb);

    /**
     * @brief 
     * @param fd 
     * @param events 
     */
    void del_fd_event(int fd, Event events);

private:
    /**
     * @brief fd context
     * @details every fd context store all event and their callback
     */
    struct FdContext {
        typedef std::shared_ptr<FdContext> ptr;

        /**
         * @brief add event and callback to dispatcher
         * @param event add event index
         * @param callback event callback
         */
        void add_event(Event event, Scheduler::ptr sched, std::function<void()> callback);
        
        /**
         * @brief del event from dispatcher
         * @param event del event index
         */
        void del_event(Event event);

        /**
         * @brief Get the event object
         */
        Event get_events();

        /**
         * @brief trigger event and call callback func
         * @param event triger event index
         */
        void trigger_event(Event event);

        /**
         * @brief clear all event and dispatch
         */
        void clear_event();

        /**
         * @brief event context
         * @details every fd contains diff read/write event
         *          need to use context to store event callback
         */
        struct EventContext {
            EventContext(Scheduler::ptr sched, std::function<void()> cb) {
                scheduler = sched;
                fiber = Fiber::ptr(new Fiber(cb, 0, sched->is_scheduler_fiber(), "fd fiber"));
            }
            typedef std::shared_ptr<EventContext> ptr;
            /// event scheduler
            Scheduler::ptr scheduler {nullptr};
            /// callback fiber
            Fiber::ptr fiber {nullptr};
        };
        /// event fd
        int fd {0};
        // dispatcher mutex
        MutexType mutex_ {};
        /// event dispatch
        std::map<Event, EventContext::ptr> dispatcher;
    };

private:
    /// epoll create fd
    int epfd_ {0};
    /// fd context list
    std::vector<FdContext::ptr> fd_ctxs_;
    /// fd mutex
    MutexType mutex_ {};
};




}

#endif