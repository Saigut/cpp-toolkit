#include <mod_worker/mod_worker.h>


/*
 * class Worker
 */
void Worker::run()
{
    while(true) {
        if (0 != do_cur_work()) {
            std::this_thread::sleep_for(std::chrono::nanoseconds(1));
        }
    }
}

int Worker::add_work(Work* work)
{
    if (!works_q.push(work)) {
        log_info("works_q.push failed!");
        return -1;
    }
    work->set_main_worker(this);
    return 0;
}

int Worker::do_cur_work()
{
    Work* cur_work = get_cur_work();
    if (cur_work) {
        cur_work->do_my_part();
        return 0;
    } else {
        return -1;
    }
}

Work* Worker::get_cur_work()
{
    Work* cur_work;
    if (works_q.pop(cur_work)) {
        return cur_work;
    } else {
        return nullptr;
    }
}
