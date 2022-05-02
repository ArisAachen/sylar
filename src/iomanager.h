#ifndef __SYLAR_SRC_IOMANAGER_H__
#define __SYLAR_SRC_IOMANAGER_H__

#include "fiber.h"
#include "scheduler.h"

#include <functional>
#include <memory>
#include <map>

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
    };

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
        void add_event(Event event, std::function<void()> callback);
        
        /**
         * @brief del event from dispatcher
         * @param event del event index
         */
        void del_event(Event event);

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
            typedef std::shared_ptr<EventContext> ptr;
            /// event scheduler
            Scheduler::ptr scheduler {nullptr};
            /// callback fiber
            Fiber::ptr fiber {nullptr};
            /// event callback
            std::function<void()> cb {nullptr};
        };
        /// event fd
        int fd {0};
        /// event dispatch
        std::map<Event, EventContext> dispatcher;
    };



private:
    /// epoll create fd
    int epfd_ {0};
};




}

#endif