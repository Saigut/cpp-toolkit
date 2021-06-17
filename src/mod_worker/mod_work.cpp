#include <mod_worker/mod_work.h>


/*
 * class Work
 */
void Work::do_my_part(int yield_param)
{
    if (stopped) {
        return;
    }
    if (!began) {
        began = true;
        std::shared_ptr<Work> cur_work = shared_from_this();
        m_wp.set_wp(context::callcc(
                [cur_work](context::continuation && c) {
                    cur_work->m_wp.set_wp(std::move(c));
                    cur_work->do_work();
                    cur_work->stopped = true;
                    cur_work->finish_handler();
                    return std::move(cur_work->m_wp.m_wp);
                }));
    } else {
        m_wp.wp_yield(yield_param);
    }
}

void Work::set_main_worker(Worker* main_worker)
{
    m_main_worker = main_worker;
}

void Work::finish_handler()
{
    if (m_consignor_work) {
        m_consignor_work->add_self_back_to_main_worker(shared_from_this());
        m_consignor_work = nullptr;
    }
}

void Work::do_work()
{
}

void Work::add_self_back_to_main_worker(std::shared_ptr<Work> sub_work)
{
    if (m_main_worker) {
        m_main_worker->add_work(new WorkWrap(shared_from_this(), sub_work));
    }
}
