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
class WorkWrap;
#include "mod_work.h"

class Worker {
public:
    virtual void run();
    virtual int add_work(WorkWrap* work);
    virtual void wait_worker_started() {};
protected:
    int do_cur_work();
    WorkWrap* get_cur_work();
    boost::lockfree::queue<WorkWrap*, boost::lockfree::capacity<5001>> works_q;
    std::mutex m_thread_lock;
};

#endif //CPP_TOOLKIT_MOD_WORKER_H
