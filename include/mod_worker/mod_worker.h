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
    WorkWrap(Work* work)
            : m_work(work)
    {}
    WorkWrap(Work* work, Work* sub_work, int sub_work_ret, bool ret_by_sub_work)
            : m_work(work), m_sub_work(sub_work),
              m_sub_work_ret(sub_work_ret), m_ret_by_sub_work(ret_by_sub_work)
    {}
    Work* m_work = nullptr;
    Work* m_sub_work = nullptr;
    int m_sub_work_ret = 0;
    bool m_ret_by_sub_work = false;
//    uint64_t m_the_sub_work_id = 0;
};

class Worker {
public:
    virtual void run();
    virtual int add_work(WorkWrap& work);
    virtual void wait_worker_started() {};
protected:
    int do_cur_work();
    int get_cur_work(WorkWrap& work);
    boost::lockfree::queue<WorkWrap, boost::lockfree::capacity<5001>> works_q;
    std::mutex m_thread_lock;
};

#endif //CPP_TOOLKIT_MOD_WORKER_H
