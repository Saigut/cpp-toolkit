#include <mod_coroutine/mod_co_mutex.hpp>
#include <boost/context/continuation.hpp>


namespace context = boost::context;

bool cppt_co_mutex_t::lock()
{
    bool expected = false;
    if (m_lock_acquired.compare_exchange_strong(expected, true)) {
        return true;
    }

    auto executor = cppt_get_cur_executor();
    auto ret_co = context::callcc([&](context::continuation && c) {
        auto caller_c = std::make_shared<context::continuation>(std::move(c));
        cppt_co_mutex_wait_queue_ele_t ele = { caller_c, executor->m_tq_idx };
        if (!m_wait_cos.try_push(ele)) {
            /// Fixme: how to do then queue is full?
            log_error("m_wait_cos queue is full!!!");
            return std::move(executor->m_executor_c);
        }
        expected = false;
        if (m_lock_acquired.compare_exchange_strong(expected, true)) {
            if (*caller_c) {
                return std::move(*caller_c);
            }
        }
        return std::move(executor->m_executor_c);
    });
    if (!ret_co) {
        // caller resume by self
        return true;
    }

    executor->m_executor_c = std::move(ret_co);
    return true;
}

bool cppt_co_mutex_t::try_lock()
{
    bool expected = false;
    return m_lock_acquired.compare_exchange_strong(expected, true);
}

void cppt_co_mutex_t::unlock()
{
    m_lock_acquired = false;
    cppt_co_mutex_wait_queue_ele_t ele;
    while (m_wait_cos.try_pop(ele)) {
        if (*(ele.c)) {
            cppt_co_add_c_ptr(ele.c, ele.tq_idx);
        }
    }
}
