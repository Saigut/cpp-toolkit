#ifndef CPP_TOOLKIT_MOD_WORKER_HPP
#define CPP_TOOLKIT_MOD_WORKER_HPP

#include <boost/lockfree/queue.hpp>
#include <memory>
#include <set>
#include <queue>
#include <mutex>
#include <chrono>
#include <thread>

#include <boost/context/continuation.hpp>
//#include <mod_atomic_queue/atomic_queue.h>


class Worker;
#include "mod_work.hpp"

struct WorkWrap {
    WorkWrap() = default;
    explicit WorkWrap(std::shared_ptr<Work> work)
    : m_work(work)
    {}
    WorkWrap(std::shared_ptr<Work> work, std::shared_ptr<Work> sub_work)
    : m_work(work), m_sub_work(sub_work)
    {}
    std::shared_ptr<Work> m_work = nullptr;
    std::shared_ptr<Work> m_sub_work = nullptr;
//    int m_sub_work_ret = 0;
};

//using AQueue = atomic_queue::AtomicQueue2<std::shared_ptr<WorkWrap>, 65535>;

class Worker {
public:
    virtual void run();
    virtual int add_work(WorkWrap* work);
    virtual void wait_worker_started() {};
protected:
    int do_cur_work();
    WorkWrap* get_cur_work();
    boost::lockfree::queue<WorkWrap*, boost::lockfree::capacity<5001>> works_q;
//    std::mutex m_thread_lock;
};

//class cppt_executor;
//class cppt_task : public std::enable_shared_from_this<cppt_task> {
//public:
//    explicit cppt_task();
//
//    void execute(int resume_param);
//
//    cppt_executor& get_executor() {
//        return m_executor;
//    }
//
//    int task_body();
//
//protected:
//    WorkingPoint m_wp;
//    cppt_executor& m_executor;
//    std::function<void(int)> finish_handler;
//
//    bool began = false;
//    bool stopped = false;
//};
//
//using AQueue = atomic_queue::AtomicQueue2<std::shared_ptr<cppt_task>, 65535>;
//
//class cppt_executor {
//public:
//    void add_task(cppt_task& task) {
//        if (!m_tasks_q.try_push(task)) {
//            log_error("cppt_executor failed to push task!");
//        }
//    }
//    bool executor_step() {
//        std::shared_ptr<cppt_task> task;
//        if (m_tasks_q.try_pop(task)) {
//            task->execute(0);
//            return true;
//        }
//        return false;
//    }
//    AQueue m_tasks_q;
//};



#endif //CPP_TOOLKIT_MOD_WORKER_HPP
