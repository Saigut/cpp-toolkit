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

int Worker::add_work(std::shared_ptr<WorkWrap> work)
{
    work->m_work->set_main_worker(this);
    {
        std::lock_guard<std::mutex> lock(m_thread_lock);
        works_q.push(work);
    }
    return 0;
}

int Worker::do_cur_work()
{
    std::shared_ptr<WorkWrap> cur_work = get_cur_work();
    if (cur_work) {
        int ret = 0;
        if (cur_work->m_sub_work) {
            if (cur_work->m_work->m_my_sub_work_msg_id != cur_work->m_sub_work->m_my_msg_id) {
                return 0;
            } else {
                ret = cur_work->m_sub_work->m_my_finish_ret_val;
            }
        }
        cur_work->m_work->m_my_sub_work_msg_id++;
        cur_work->m_work->do_my_part(ret);
        return 0;
    } else {
        return -1;
    }
}

std::shared_ptr<WorkWrap> Worker::get_cur_work()
{
    std::lock_guard<std::mutex> lock(m_thread_lock);
    if (works_q.empty()) {
        return nullptr;
    }
    std::shared_ptr<WorkWrap> cur_work = works_q.front();
    works_q.pop();
    return cur_work;
}
