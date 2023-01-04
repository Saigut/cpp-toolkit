#include <mod_coroutine/mod_coroutine.h>

#include <queue>
#include <map>
#include <memory>
#include <thread>
#include <condition_variable>
#include <boost/context/continuation.hpp>
#include <boost/asio/io_context.hpp>
#include <mod_atomic_queue/atomic_queue.h>
#include <boost/asio/deadline_timer.hpp>
#include <mod_np_queue/mod_np_queue.hpp>


// Types
namespace context = boost::context;
using boost::asio::io_context;

class cppt_task_t {
public:
    cppt_co_sp m_co_wrapper = nullptr;
    std::function<void()> m_f_before_execution = nullptr;
    std::function<void(cppt_co_sp)> m_f_after_execution = nullptr;
};


// Values
static std::vector<np_queue_t<cppt_task_t>> g_task_queues(1);
static thread_local unsigned tq_idx = 0;
static thread_local context::continuation g_executor_c;
static thread_local cppt_task_t g_cur_task_c;

bool g_run_flag = true;
static unsigned gs_core_num = 1;
static unsigned gs_use_core = 0;

static io_context g_io_ctx;


// Helpers
static void cppt_co_add_task(cppt_task_t&& task)
{
    unsigned core_cnt = 0;
    do {
        if (g_task_queues[gs_use_core++ % gs_core_num]
                .try_enqueue(std::move(task))) {
            return;
        }
        core_cnt++;
    } while (core_cnt <= gs_core_num);
    log_error("coroutine task queues full!");
}
static void cppt_co_add_ptr(cppt_co_sp wrapper)
{
    cppt_task_t task;
    task.m_co_wrapper = wrapper;
    cppt_co_add_task(std::move(task));
}
static void cppt_co_add_c_ptr(cppt_co_c_sp c)
{
    auto new_co = std::make_shared<cppt_co_t>(c);
    cppt_co_add_ptr(new_co);
}

static void cppt_co_add_sptr_f_before(cppt_co_sp wrapper,
                                      std::function<void()>& f_before)
{
    cppt_task_t task;
    task.m_co_wrapper = wrapper;
    task.m_f_before_execution = f_before;
    cppt_co_add_task(std::move(task));
}

// Type implementation
void cppt_co_t::start_user_co()
{
    *m_c = context::callcc([&](context::continuation && c) {
        g_executor_c = std::move(c);
        m_co_started = true;
        m_user_co();
        m_co_stopped = true;
        cppt_co_c_sp waiting_c;
        while (m_wait_cos.try_pop(waiting_c)) {
            if (*waiting_c) {
                cppt_co_add_c_ptr(waiting_c);
            }
        }
        return std::move(g_executor_c);
    });
}

void cppt_co_t::resume_user_co()
{
    *m_c = m_c->resume();
}

bool cppt_co_t::is_started()
{
    return m_co_started;
}

bool cppt_co_t::can_resume()
{
    return *m_c ? true : false;
}

void cppt_co_t::join()
{
    if (m_co_stopped) {
        return;
    }
    auto ret_co = context::callcc([&](context::continuation && c) {
        auto caller_c = std::make_shared<context::continuation>(std::move(c));
        if (!m_wait_cos.try_push(caller_c)) {
            /// Fixme: how to do then queue is full?
            log_error("m_wait_cos queue is full!!!");
            return std::move(g_executor_c);
        }
        if (m_co_stopped) {
            if (*caller_c) {
                return std::move(*caller_c);
            }
        }
        return std::move(g_executor_c);
    });
    if (!ret_co) {
        // caller resume by self
        return;
    }
    g_executor_c = std::move(ret_co);
}


// Interfaces
cppt_co_sp cppt_co_create0(std::function<void()> user_co)
{
    cppt_task_t task;
    auto co_wrapper = std::make_shared<cppt_co_t>(std::move(user_co));
    task.m_co_wrapper = co_wrapper;
    cppt_co_add_task(std::move(task));
    return co_wrapper;
}

static void asio_thread(io_context& io_ctx)
{
    util_thread_set_self_name("asio thr");
    boost::asio::io_context::work io_work(io_ctx);
    io_ctx.run();
    log_info("Asio io context quit!!");
}

static void cppt_co_main_run_thread(unsigned core_idx)
{
    char thr_name[8];
    util_bind_thread_to_core(core_idx);
    snprintf(thr_name, sizeof(thr_name), "co%02u", core_idx);
    util_thread_set_self_name(thr_name);

    tq_idx = core_idx;

    std::mutex cond_lock;
    std::condition_variable cond_cv;

    auto notify_handler =
            [&]()
    {
        std::lock_guard lock(cond_lock);
        cond_cv.notify_one();
    };

    auto wait_handler =
            [&]()
    {
        {
            std::unique_lock<std::mutex> u_lock(cond_lock);
            cond_cv.wait(u_lock);
        }
        return g_run_flag;
    };

    g_task_queues[tq_idx].set_handlers(notify_handler, wait_handler);

    while (g_run_flag) {
        if (g_task_queues[tq_idx].dequeue(g_cur_task_c)) {
            auto co_wrapper = g_cur_task_c.m_co_wrapper;
            if (!co_wrapper->is_started()) {
                co_wrapper->start_user_co();
            } else if (co_wrapper->can_resume()) {
                if (g_cur_task_c.m_f_before_execution) {
                    g_cur_task_c.m_f_before_execution();
                    g_cur_task_c.m_f_before_execution = nullptr;
                }
                co_wrapper->resume_user_co();
            } else {
                g_cur_task_c.m_co_wrapper = nullptr;
                continue;
            }
            if (g_cur_task_c.m_f_after_execution) {
                g_cur_task_c.m_f_after_execution(g_cur_task_c.m_co_wrapper);
                g_cur_task_c.m_f_after_execution = nullptr;
            } else {
                g_cur_task_c.m_co_wrapper = nullptr;
            }
        }
    }
    log_info("Thread %s quit.", thr_name);
}

void cppt_co_main_run()
{
    // io and timer thread
    std::thread asio_thr{ asio_thread, std::ref(g_io_ctx) };
    asio_thr.detach();

    // prepare coroutine threads
    auto core_num = std::thread::hardware_concurrency();
    if (0 == core_num) {
        gs_core_num = 1;
    } else {
        gs_core_num = core_num;
    }
    for (unsigned i = 1; i < gs_core_num; i++) {
        std::thread t{cppt_co_main_run_thread, i};
        t.detach();
    }
    cppt_co_main_run_thread(0);
}

// ret: 0, ok; -1 coroutine error
int cppt_co_yield(
        const std::function<void(std::function<void()>&&)>& wrapped_extern_func)
{
    if (!g_executor_c) {
        return -1;
    }
    auto wrap_func = [&](cppt_co_sp co_wapper) {
        wrapped_extern_func([co_wapper](){
            /// Fixme: how to do when executing queue is full?
            cppt_co_add_ptr(co_wapper);
        });
    };
    g_cur_task_c.m_f_after_execution = wrap_func;
    g_executor_c = g_executor_c.resume();
    return 0;
}

// ret: 0, ok; 1 timeout; -1 coroutine error
int cppt_co_yield_timeout(
        const std::function<void(std::function<void()>&&)>& wrapped_extern_func,
        unsigned int timeout_ms,
        std::function<void()>& f_cancel_operation)
{
    if (!g_executor_c) {
        return -1;
    }
    bool is_timeout = false;

    boost::asio::deadline_timer timer{ g_io_ctx };

    auto wrap_func = [&](cppt_co_sp co_wapper) {
        wrapped_extern_func([&, co_wapper](){
            std::function<void()> f_before = [&](){
                // stop timer
                timer.cancel();
            };
            /// Fixme: how to do when executing queue is full?
            cppt_co_add_sptr_f_before(co_wapper, f_before);
        });
        timer.expires_from_now(boost::posix_time::milliseconds(timeout_ms));
        timer.async_wait([&, co_wapper](const boost::system::error_code& ec){
            if (ec) {
                return;
            }
            std::function<void()> f_before = [&](){
                is_timeout = true;
                // cancel operation
                if (f_cancel_operation) {
                    f_cancel_operation();
                }
            };
            /// Fixme: how to do when executing queue is full?
            cppt_co_add_sptr_f_before(co_wapper, f_before);
        });
    };

    g_cur_task_c.m_f_after_execution = wrap_func;
    g_executor_c = g_executor_c.resume();

    return is_timeout ? 1 : 0;
}
