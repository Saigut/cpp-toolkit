#ifndef CPP_TOOLKIT_MOD_WORKER_H
#define CPP_TOOLKIT_MOD_WORKER_H

#include <boost/lockfree/queue.hpp>
#include <memory>
#include <set>
#include <queue>
#include <mutex>
#include <chrono>
#include <thread>

#include <boost/context/continuation.hpp>


class Worker;
#include "mod_work.h"

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

class Task {
public:
    void execute();
protected:
    int task();
    void finish_task(int task_ret);
};

class Executor {
public:
    void execute_tasks();
    void add_task(Task task);
};



#endif //CPP_TOOLKIT_MOD_WORKER_H
