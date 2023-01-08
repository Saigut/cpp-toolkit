#ifndef CPP_TOOLKIT_MOD_CO_MUTEX_HPP
#define CPP_TOOLKIT_MOD_CO_MUTEX_HPP

#include <atomic>
#include <mod_coroutine/mod_coroutine.h>


struct cppt_co_mutex_wait_queue_ele_t {
    cppt_co_c_sp c;
    unsigned tq_idx;
};
using cppt_co_mutex_wait_queue_t = atomic_queue::AtomicQueue2<cppt_co_mutex_wait_queue_ele_t, 1024>;


class cppt_co_mutex_t {
public:
    bool lock();
    bool try_lock();
    void unlock();

private:
    std::atomic<bool> m_lock_acquired = false;
    cppt_co_mutex_wait_queue_t m_wait_cos;
};

#endif //CPP_TOOLKIT_MOD_CO_MUTEX_HPP
