#ifndef CPP_TOOLKIT_WORKER_H
#define CPP_TOOLKIT_WORKER_H

#include <boost/lockfree/queue.hpp>
#include <memory>
#include <set>
#include <queue>
#include <mutex>
#include <chrono>
#include <thread>
//#include "thing.h"

#include <boost/context/continuation.hpp>



template< typename Th >
class Worker {
public:
    Worker() = default;
    ~Worker() {}
    void run();

    // things available to do
    int add_avail_thing(Th* thing);

    // things waiting other to do
    int notify_thing_other_part_done(Th* thing);

protected:
    // things available to do
    int do_cur_avail_thing();

    Th* get_cur_avail_thing();

protected:
//    boost::lockfree::queue<Th*> avail_things_q;
//    std::queue<Th*> avail_things_q;
    boost::lockfree::queue<Th*> avail_things_q;
//    std::set<Th*> waiting_things_set;

    std::mutex mutex_lock;
};

template< typename Th >
void Worker<Th>::run()
{
    while(true) {
        if (0 != do_cur_avail_thing()) {
            std::this_thread::sleep_for(std::chrono::nanoseconds(1));
        }
    }
}

template< typename Th >
int Worker<Th>::add_avail_thing(Th* thing)
{
    avail_things_q.push(thing);
    thing->set_main_worker(this);
    return 0;
}

template< typename Th >
int Worker<Th>::notify_thing_other_part_done(Th* thing)
{
    thing->notify_other_part_done();
    // if not finished, avail_things_q push thing
    return 0;
}


template< typename Th >
int Worker<Th>::do_cur_avail_thing()
{
    Th* cur_thing = get_cur_avail_thing();
    if (cur_thing) {
        cur_thing->do_main_part();
        return 0;
    } else {
        return -1;
    }
}

template< typename Th >
Th* Worker<Th>::get_cur_avail_thing()
{
    Th* cur_thing;
    if (avail_things_q.pop(cur_thing)) {
//        log_info("q %s", avail_things_q.empty() ? "empty" : "not empty");
        return cur_thing;
    } else {
        return nullptr;
    }
}


#endif //CPP_TOOLKIT_WORKER_H
