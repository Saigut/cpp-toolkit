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
protected:
    int do_cur_work();
    Work* get_cur_work();
    boost::lockfree::queue<Work*, boost::lockfree::capacity<100>> works_q;
};

#endif //CPP_TOOLKIT_MOD_WORKER_H
