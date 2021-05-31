#ifndef CPP_TOOLKIT_WORKER_H
#define CPP_TOOLKIT_WORKER_H

#include <boost/lockfree/queue.hpp>
#include <memory>
#include <set>
#include <mutex>
#include <chrono>
#include <thread>
#include "thing.h"

#include <boost/context/continuation.hpp>

class Worker {
public:

    void run() {
        while(true) {
            if (0 != do_cur_avail_thing()) {
                std::this_thread::sleep_for(std::chrono::nanoseconds(1));
            }
        }
    }

    // things available to do
    int add_avail_thing(SP_Thing_t thing) {
        avail_things_q.push(thing);
    }

    // things waiting other to do
    int notify_thing_other_part_done(SP_Thing_t thing) {
        thing->notify_other_part_done();
        // if not finished, avail_things_q push thing
    }

protected:
    // things available to do
    int do_cur_avail_thing() {
        SP_Thing_t cur_thing = get_cur_avail_thing();
        if (cur_thing) {
            cur_thing->do_main_part();
            return 0;
        } else {
            return -1;
        }
    }

    SP_Thing_t get_cur_avail_thing() {
        SP_Thing_t cur_thing;
        avail_things_q.pop(cur_thing);
        return cur_thing;
    }

protected:
    boost::lockfree::queue<SP_Thing_t> avail_things_q;
    std::set<SP_Thing_t> waiting_things_set;

    std::mutex mutex_lock;
};

#endif //CPP_TOOLKIT_WORKER_H
