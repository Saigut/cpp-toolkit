#include <mod_worker/mod_worker.h>

#include <mutex>
#include <mod_common/expect.h>

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

int Worker::add_work(WorkWrap* work)
{
//    std::lock_guard<std::mutex> lock(m_thread_lock);
    work->m_work->set_main_worker(this);
    if (!works_q.push(work)) {
        log_info("works_q.push failed!");
        return -1;
    }
    return 0;
}

int Worker::do_cur_work()
{
    WorkWrap* cur_work = get_cur_work();
    if (cur_work) {
        if (cur_work->m_sub_work) {
            if (!(cur_work->m_sub_work->dealed_with_finish)) {
                cur_work->m_sub_work->finish_ret = cur_work->m_sub_work_ret;
                cur_work->m_work->do_my_part();
                cur_work->m_sub_work->dealed_with_finish = true;
            } else {
                /// fixme   Warning: When to release resource of sub_work?
                delete cur_work->m_sub_work;
            }
        } else {
            cur_work->m_work->do_my_part();
        }
        delete cur_work;
        return 0;
    } else {
        return -1;
    }
}

WorkWrap* Worker::get_cur_work()
{
//    std::lock_guard<std::mutex> lock(m_thread_lock);
    WorkWrap* cur_work;
    if (works_q.pop(cur_work)) {
        return cur_work;
    } else {
        return nullptr;
    }
}
