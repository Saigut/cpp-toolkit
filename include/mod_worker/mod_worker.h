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

class Worker {
public:
    virtual void run();
    virtual int add_work(Work* work);
    int do_cur_work();
    Work* get_cur_work();
protected:
    boost::lockfree::queue<Work*, boost::lockfree::capacity<5001>> works_q;
    std::mutex m_thread_lock;
};

#endif //CPP_TOOLKIT_MOD_WORKER_H
