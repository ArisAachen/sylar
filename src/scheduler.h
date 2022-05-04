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

namespace sylar {


class Scheduler : public std::enable_shared_from_this<Scheduler> {
public:
    typedef std::shared_ptr<Scheduler> ptr;
    typedef std::weak_ptr<Scheduler> weak_ptr;

    typedef Mutex MutexType;
    typedef ConditionBlock ConditionType;
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
    virtual void start();

    /**
     * @brief stop scheduler
     */
    virtual void stop();

    /**
     * @brief schedule func
     * @param cb fiber func
     * @param thread thread id
     */
    virtual void schedule(std::function<void()> cb, int thread = -1);

    /**
     * @brief schedule func
     * @param fiber fiber
     * @param thread thread id
     */
    virtual void schedule(Fiber::ptr fiber = nullptr, int thread = -1);

    /**
     * @brief check if current fiber is in main thread 
     */
    static bool is_scheduler_fiber();

    /**
     * @brief Get the schedule fiber object
     */
    static Fiber::ptr get_schedule_fiber();

protected:
    /**
     * @brief run scheduler
     */
    void run();

    /**
     * @brief idle scheduler
     */
    virtual void idle();

private:
    /**
     * @brief schedule task item
     */
    struct ScheduleTask {
        typedef std::shared_ptr<ScheduleTask> ptr;

        /**
         * @brief Construct a new Schedule Task object
         */
        ScheduleTask() {}

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
        ScheduleTask(std::function<void()> f, bool use_caller = true, int thr = -1) {
            fiber = Fiber::ptr(new Fiber(f, 0, use_caller));
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

        /**
         * @brief execute fiber
         */
        void execute() {
            if (fiber != nullptr)
                fiber->resume();
        }

        /// execute fiber
        Fiber::ptr fiber;
        /// thread id
        int thread {0};
    };

private:
    /**
     * @brief check is tasks is empty
     */
    bool is_tasks_empty();

    /**
     * @brief push task into list end
     * @param task append task
     */
    void task_push(ScheduleTask::ptr task);

    /**
     * @brief pop task from end, if task is empty, call idle fiber
     */
    ScheduleTask::ptr task_pop();

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
    MutexType mutex_ {};
    /// condition type
    ConditionType cond_ {mutex_};
};













}



#endif