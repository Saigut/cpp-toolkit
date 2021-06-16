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

int Worker::add_work(WorkWrap& work)
{
//    std::lock_guard<std::mutex> lock(m_thread_lock);
    work.m_work->set_main_worker(this);
    if (!works_q.push(work)) {
        log_info("works_q.push failed!");
        return -1;
    }
    return 0;
}

int Worker::do_cur_work()
{
    WorkWrap cur_work;
    int ret = get_cur_work(cur_work);
    if (0 == ret) {
        if (cur_work.m_sub_work
            && (cur_work.m_sub_work->m_my_msg_id != cur_work.m_work->m_my_sub_work_msg_id)) {
            delete cur_work.m_sub_work;
            return 0;
        }
        cur_work.m_work->m_my_sub_work_msg_id++;
        cur_work.m_work->do_my_part(cur_work.m_sub_work_ret);
        return 0;
    } else {
        return -1;
    }
}

int Worker::get_cur_work(WorkWrap& cur_work)
{
//    std::lock_guard<std::mutex> lock(m_thread_lock);
    if (works_q.pop(cur_work)) {
        return 0;
    } else {
        return -1;
    }
}
