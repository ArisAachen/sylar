#ifndef __SYLAR_SRC_SCHEDULER_H__
#define __SYLAR_SRC_SCHEDULER_H__

#include "mutex.h"
#include "fiber.h"
#include "thread.h"

#include <list>
#include <memory>
#include <string>
#include <vector>
#include <functional>
#include <condition_variable>

namespace sylar {


class Scheduler  {
public:
    typedef std::shared_ptr<Scheduler> ptr;
    typedef Mutex MutexType;
    /**
     * @brief Construct a new Scheduler object
     * @param threads thread numeber
     * @param use_caller if use caller as scheduler
     * @param name scheduler name 
     */
    Scheduler(size_t threads = 1, bool use_caller = true, const std::string& name = "Scheduler");

    /**
     * @brief Destroy the virtual Scheduler object
     */
    virtual~Scheduler();

    /**
     * @brief start scheduler
     */
    void start();

    /**
     * @brief stop scheduler
     */
    void stop();

protected:
    /**
     * @brief run scheduler
     */
    void run();

private:
    /**
     * @brief schedule task item
     */
    struct ScheduleTask {
        typedef std::shared_ptr<ScheduleTask> ptr;
        /**
         * @brief Construct a new Schedule Task object
         * @param f fiber func
         * @param thr thread id
         */
        ScheduleTask(Fiber::ptr f, int thr): fiber(f), thread(thr) {}

        /**
         * @brief Construct a new Schedule Task object
         * @param f execute func
         * @param thr thread id 
         */
        ScheduleTask(std::function<void()> f, int thr = -1) {
            fiber = Fiber::ptr(new Fiber(f, 512, true));
            thread = thr;
        }

        /**
         * @brief reset fiber
         * @param f fiber 
         * @param thr thread id
         */
        void reset(Fiber::ptr f, int thr) {
            fiber->yield();
            fiber.swap(f);
            thread = thr;
        }

        /**
         * @brief reset fiber
         * @param f fiber 
         * @param thr thread id
         */
        void reset(std::function<void()> f, int thr = -1) {
            fiber->reset(f);
            thread = thr;
        }

        /// execute fiber
        Fiber::ptr fiber;
        /// call func
        std::function<void()> cb;
        /// thread id
        int thread {0};
    };

private:
    /// scheduler name 
    std::string name_ {""};
    /// thread num
    int thread_num_ {0};
    /// thread pool
    std::vector<Thread::ptr> threads_;
    /// schedule task list
    std::list<ScheduleTask::ptr> tasks_;
    /// thread id vector
    std::vector<int> thread_ids_;
    /// use main thread as caller
    bool use_caller_ {true};
    /// run state
    bool running_ {false};
    /// mutex type
    MutexType mutex_;
    // 
    std::condition_variable block_;
};













}



#endif