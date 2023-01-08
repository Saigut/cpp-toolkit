#ifndef CPP_TOOLKIT_MOD_COROUTINE_H
#define CPP_TOOLKIT_MOD_COROUTINE_H

#include <functional>
#include <thread>
#include <mod_common/utils.h>
#include <boost/context/continuation.hpp>
#include <mod_atomic_queue/atomic_queue.h>

using cppt_co_c_sp = std::shared_ptr<boost::context::continuation>;
struct cppt_co_wait_queue_ele_t {
    cppt_co_c_sp c;
    unsigned tq_idx;
};
using cppt_co_wait_queue_t = atomic_queue::AtomicQueue2<cppt_co_wait_queue_ele_t, 100>;

class cppt_co_t {
public:
    explicit cppt_co_t(std::function<void()> user_co)
            : m_c(std::make_shared<boost::context::continuation>()),
              m_user_co(std::move(user_co)) {}
    explicit cppt_co_t(cppt_co_c_sp c)
            : m_c(c), m_co_started(true) {}

    bool is_started();
    bool can_resume();

    void start_user_co();
    void resume_user_co();

    void join();

private:
    cppt_co_c_sp m_c;
    std::function<void()> m_user_co;
    bool m_co_started = false;
    std::atomic<bool> m_co_stopped = false;
    cppt_co_wait_queue_t m_wait_cos;
};
using cppt_co_sp = std::shared_ptr<cppt_co_t>;


cppt_co_sp cppt_co_create0(std::function<void()> user_co);

template<typename Function, typename... Args>
cppt_co_sp cppt_co_create(Function& f, Args... args)
{
    auto params = std::make_tuple(std::forward<Args>(args)...);
    auto user_co = [=](){
        call_with_variadic_arg(f, params);
    };
    return cppt_co_create0(std::move(user_co));
}

template<typename Function, typename... Args>
cppt_co_sp cppt_co_create(Function&& f, Args... args)
{
    auto params = std::make_tuple(std::forward<Args>(args)...);
    auto user_co = [=](){
        call_with_variadic_arg(f, params);
    };
    return cppt_co_create0(std::move(user_co));
}


void cppt_co_main_run();

// ret: 0, ok; -1 coroutine error
int cppt_co_yield(
        const std::function<void(std::function<void()>&&)>& wrapped_extern_func);
// ret: 0, ok; 1 timeout
int cppt_co_yield_timeout(
        const std::function<void(std::function<void()>&&)>& wrapped_extern_func,
        unsigned int timeout_ms,
        const std::function<void()>& f_cancel_operation);


class cppt_task_t {
public:
    cppt_co_sp m_co = nullptr;
    std::function<void()> m_f_before_execution = nullptr;
    std::function<void(cppt_co_sp)> m_f_after_execution = nullptr;
};

class co_executor_t;
using co_executor_sp = std::shared_ptr<co_executor_t>;
class co_executor_t {
public:
    co_executor_t() = default;
    explicit co_executor_t(unsigned tq_idx) : m_tq_idx(tq_idx) {}
    void start(co_executor_sp executor);

    bool m_turn_on = false;
    bool m_is_executing = false;
    bool m_is_blocking = false;

    unsigned m_tq_idx = 0;
    boost::context::continuation m_executor_c;
    cppt_task_t m_cur_task_c;
    std::thread m_t;
};

co_executor_sp cppt_get_cur_executor();
void cppt_co_add_c_ptr(cppt_co_c_sp c, unsigned tq_idx);

#endif //CPP_TOOLKIT_MOD_COROUTINE_H
