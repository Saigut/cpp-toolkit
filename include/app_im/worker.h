#ifndef CPP_TOOLKIT_WORKER_H
#define CPP_TOOLKIT_WORKER_H

#include <boost/lockfree/queue.hpp>
#include <memory>
#include <set>
#include <queue>
#include <mutex>
#include <chrono>
#include <thread>

#include <boost/context/continuation.hpp>

class Worker;
class Worker_Net;
#include "work.h"

class Worker {
public:
    virtual void run();
    virtual int add_work(Work* work);
protected:
    int do_cur_work();
    Work* get_cur_work();
    boost::lockfree::queue<Work*, boost::lockfree::capacity<100>> works_q;
};

class Worker_Net : public Worker {
public:
    void run() override;
    int add_work(Work* work) override { exit(-1); return -1; };
    int add_avail_work(Work_GetStdin* work);
    int add_avail_work(Work_NetIn* work);
    int add_avail_work(Work_NetOut* work);
private:
    int m_epoll_fd = -1;
};

#endif //CPP_TOOLKIT_WORKER_H
