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

void Task::execute() {
    finish_task(task());
}

int Task::task() {
    return 0;
}

void Task::finish_task(int task_ret) {

}

void Executor::execute_tasks() {

}

void Executor::add_task(Task task) {

}
