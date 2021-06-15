#include <mod_worker/mod_work.h>


/*
 * class Work
 */
void Work::do_my_part()
{
    if (stopped) {
        return;
    }
    if (!began) {
        began = true;
        class Work* t_this = this;
        m_wp.set_wp(context::callcc(
                [t_this](context::continuation && c) {
                    t_this->m_wp.set_wp(std::move(c));
                    t_this->do_work();
                    t_this->stopped = true;
                    t_this->finish_handler(t_this, 0, true);
                    return std::move(t_this->m_wp.m_wp);
                }));
    } else {
        m_wp.yield();
    }
}

void Work::set_main_worker(Worker* main_worker)
{
    m_main_worker = main_worker;
}

void Work::finish_handler(Work* sub_work, int sub_work_ret, bool ret_by_sub_work)
{
    if (m_consignor_work) {
        m_consignor_work->add_self_back_to_main_worker(sub_work, sub_work_ret, ret_by_sub_work);
        m_consignor_work = nullptr;
    }
}

void Work::do_work()
{
}

void Work::add_self_back_to_main_worker(Work* sub_work, int sub_work_ret, bool ret_by_sub_work)
{
    if (m_main_worker) {
        auto work_connect_wrap = new WorkWrap(this, sub_work, sub_work_ret, ret_by_sub_work);
        m_main_worker->add_work(work_connect_wrap);
    }
}
