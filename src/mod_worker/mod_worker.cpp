#include <mod_worker/mod_worker.hpp>

#include <mutex>
#include <mod_common/expect.hpp>

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
    expect_ret_val(works_q.push(work), -1);
    return 0;
}

int Worker::do_cur_work()
{
    WorkWrap* cur_work = get_cur_work();
    if (cur_work) {
        int ret = 0;
        if (cur_work->m_sub_work) {
            if (cur_work->m_work->m_my_sub_work_msg_id != cur_work->m_sub_work->m_my_msg_id) {
                delete cur_work;
                return 0;
            } else {
                ret = cur_work->m_sub_work->m_my_finish_ret_val;
            }
        }
        cur_work->m_work->m_my_sub_work_msg_id++;
        cur_work->m_work->do_my_part(ret);
        delete cur_work;
        return 0;
    } else {
        return -1;
    }
}

WorkWrap* Worker::get_cur_work()
{
//    std::lock_guard<std::mutex> lock(m_thread_lock);
//    if (works_q.empty()) {
//        return nullptr;
//    }
    WorkWrap* cur_work;
    return works_q.pop(cur_work) ? cur_work : nullptr;
}

//void cppt_task::execute(int resume_param) {
//    if (stopped) {
//        return;
//    }
//    if (!began) {
//        began = true;
//        std::shared_ptr<cppt_task> task = shared_from_this();
//        m_wp.set_wp(context::callcc(
//                [task](context::continuation && c) {
//                    int ret;
//                    task->m_wp.set_wp(std::move(c));
//                    ret = task->task_body();
//                    task->stopped = true;
//                    if (task->finish_handler) {
//                        task->finish_handler(ret);
//                    }
//                    return std::move(task->m_wp.m_wp);
//                }));
//    } else {
//        m_wp.wp_yield(resume_param);
//    }
//}
//
//int cppt_task::task_body() {
//    return 0;
//}
