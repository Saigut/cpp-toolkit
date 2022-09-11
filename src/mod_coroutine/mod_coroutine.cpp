#include <mod_coroutine/mod_coroutine.h>

#include <queue>
#include <map>
#include <memory>
#include <thread>
#include <boost/context/continuation.hpp>
#include <boost/asio/io_context.hpp>
#include <mod_atomic_queue/atomic_queue.h>
#include <asio/deadline_timer.hpp>


// Types
namespace context = boost::context;
using boost::asio::io_context;

class cppt_co_wrapper {
public:
    explicit cppt_co_wrapper(std::function<void()> user_co)
            : m_c(std::make_shared<context::continuation>()),
              m_user_co(std::move(user_co)) {}
    explicit cppt_co_wrapper(std::shared_ptr<context::continuation> c)
            : m_c(c), m_co_started(true) {}
    virtual context::continuation start_user_co();
    std::shared_ptr<context::continuation> m_c;
//protected:
    std::function<void()> m_user_co;
    bool m_co_started = false;
};

class cppt_co_wrapper_awaitable : public cppt_co_wrapper {
public:
    explicit cppt_co_wrapper_awaitable(std::function<void()> user_co, uint32_t co_id)
            : cppt_co_wrapper(std::move(user_co)), m_co_id(co_id) {}
    explicit cppt_co_wrapper_awaitable(std::shared_ptr<context::continuation> c,
                                       uint32_t co_id)
            : cppt_co_wrapper(c), m_co_id(co_id) {}
    context::continuation start_user_co() override;
private:
    uint32_t m_co_id;
};

class cppt_co_exec_queue_ele {
public:
    cppt_co_wrapper* m_co_wrapper = nullptr;
    std::function<void()> m_f_before_execution = nullptr;
    std::function<void(cppt_co_wrapper*)> m_f_after_execution = nullptr;
};

using cppt_co_queue_t = atomic_queue::AtomicQueue2<cppt_co_exec_queue_ele, 65535>;


// Values
thread_local context::continuation g_cppt_co_c;
thread_local std::map<uint32_t, std::function<void()>> g_co_awaitable_map;
thread_local std::queue<uint32_t> g_awaitable_id_queue;
thread_local cppt_co_exec_queue_ele g_cur_co;
cppt_co_queue_t g_co_exec_queue;
bool g_run_flag = true;

io_context g_io_ctx;


// Helpers
static void init_awaitable_id_queue()
{
    int i;
    for (i = 0; i < 65536; i++) {
        g_awaitable_id_queue.push(i);
    }
}
void cppt_co_add_c(context::continuation&& c)
{
    auto new_co = new cppt_co_wrapper(
            std::make_shared<context::continuation>(std::move(c)));
    cppt_co_exec_queue_ele ele;
    ele.m_co_wrapper = new_co;
    if (!g_co_exec_queue.try_push(ele)) {
        log_error("g_co_exec_queue full!");
        delete new_co;
    }
}
void cppt_co_add_c_sptr(std::shared_ptr<context::continuation> c)
{
    auto new_co = new cppt_co_wrapper(c);
    cppt_co_exec_queue_ele ele;
    ele.m_co_wrapper = new_co;
    if (!g_co_exec_queue.try_push(ele)) {
        log_error("g_co_exec_queue full!");
        delete new_co;
    }
}
static void cppt_co_add_sptr(cppt_co_wrapper* wrapper)
{
    cppt_co_exec_queue_ele ele;
    ele.m_co_wrapper = wrapper;
    if (!g_co_exec_queue.try_push(ele)) {
        log_error("g_co_exec_queue full!");
        delete wrapper;
    }
}
static void cppt_co_add_sptr_f_before(cppt_co_wrapper* wrapper,
                                      std::function<void()>& f_before)
{
    cppt_co_exec_queue_ele ele;
    ele.m_co_wrapper = wrapper;
    ele.m_f_before_execution = f_before;
    if (!g_co_exec_queue.try_push(ele)) {
        log_error("g_co_exec_queue full!");
        delete wrapper;
    }
}

// Type implementation
context::continuation cppt_co_wrapper::start_user_co()
{
    return context::callcc([&](context::continuation && c) {
        g_cppt_co_c = std::move(c);
        m_user_co();
        return std::move(g_cppt_co_c);
    });
}

context::continuation cppt_co_wrapper_awaitable::start_user_co()
{
    return context::callcc([&, co_id(m_co_id)](context::continuation && c) {
        g_cppt_co_c = std::move(c);
        m_user_co();
        auto rst = g_co_awaitable_map.find(co_id);
        if (rst != g_co_awaitable_map.end()) {
            if (rst->second) {
                cppt_co_create(rst->second);
            }
            g_co_awaitable_map.erase(rst);
            g_awaitable_id_queue.push(co_id);
        }

        return std::move(g_cppt_co_c);
    });
}


// Interfaces
void cppt_co_create0(std::function<void()> user_co)
{
    cppt_co_exec_queue_ele ele;
    ele.m_co_wrapper = new cppt_co_wrapper(std::move(user_co));
    g_co_exec_queue.push(ele);
}

unsigned int cppt_co_awaitable_create0(std::function<void()> user_co)
{
    if (g_awaitable_id_queue.empty()) {
        log_error("No available await id to use!");
        return -1;
    }
    uint32_t id = g_awaitable_id_queue.front();
    g_awaitable_id_queue.pop();
    cppt_co_exec_queue_ele ele;
    ele.m_co_wrapper = new cppt_co_wrapper_awaitable(std::move(user_co), id);
    g_co_exec_queue.push(ele);
    return id;
}

static void asio_thread(io_context& io_ctx)
{
    boost::asio::io_context::work io_work(io_ctx);
    io_ctx.run();
    log_info("Asio io context quit!!");
}

void cppt_co_main_run()
{
    std::thread asio_thr{ asio_thread, std::ref(g_io_ctx) };
    asio_thr.detach();

    init_awaitable_id_queue();
    while (g_run_flag) {
        if (!g_co_exec_queue.was_empty()) {
            g_cur_co = g_co_exec_queue.pop();
            auto co_wrapper = g_cur_co.m_co_wrapper;
            if (!co_wrapper->m_co_started) {
                co_wrapper->m_co_started = true;
                *co_wrapper->m_c = std::move(co_wrapper->start_user_co());
            } else if (*co_wrapper->m_c) {
                if (g_cur_co.m_f_before_execution) {
                    g_cur_co.m_f_before_execution();
                    g_cur_co.m_f_before_execution = nullptr;
                }
                *co_wrapper->m_c = std::move(co_wrapper->m_c->resume());
            } else {
                delete g_cur_co.m_co_wrapper;
                g_cur_co.m_co_wrapper = nullptr;
                continue;
            }
            if (g_cur_co.m_f_after_execution) {
                g_cur_co.m_f_after_execution(g_cur_co.m_co_wrapper);
                g_cur_co.m_f_after_execution = nullptr;
            } else {
                delete g_cur_co.m_co_wrapper;
                g_cur_co.m_co_wrapper = nullptr;
            }
        } else {
            cppt_nanosleep(1);
        }
    }
}

void cppt_co_yield(
        const std::function<void(std::function<void()>&&)>& wrapped_extern_func)
{
    auto wrap_func = [&](cppt_co_wrapper* co_wapper) {
        wrapped_extern_func([&](){
            cppt_co_add_sptr(co_wapper);
        });
    };
    g_cur_co.m_f_after_execution = wrap_func;
    g_cppt_co_c = g_cppt_co_c.resume();
}

// ret: 0, ok; 1 timeout
int cppt_co_yield_timeout(
        const std::function<void(std::function<void()>&&)>& wrapped_extern_func,
        unsigned int timeout_ms,
        std::function<void()>& f_cancel_operation)
{
    bool is_timeout = false;

    boost::asio::deadline_timer timer{ g_io_ctx };

    auto wrap_func = [&](cppt_co_wrapper* co_wapper) {
        wrapped_extern_func([&](){
            std::function<void()> f_before = [&](){
                // stop timer
                timer.cancel();
            };
            cppt_co_add_sptr_f_before(co_wapper, f_before);
        });
        timer.expires_from_now(boost::posix_time::milliseconds(timeout_ms));
        timer.async_wait([&](const boost::system::error_code& ec){
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
            cppt_co_add_sptr_f_before(co_wapper, f_before);
        });
    };

    g_cur_co.m_f_after_execution = wrap_func;
    g_cppt_co_c = g_cppt_co_c.resume();

    return is_timeout ? 1 : 0;
}

void cppt_co_await(unsigned int co_id)
{
    auto rst = g_co_awaitable_map.find(co_id);
    if (rst != g_co_awaitable_map.end()) {
        return;
    }
    context::callcc([&](context::continuation && c) {
        auto sptr_c = std::make_shared<context::continuation>(std::move(c));
        g_co_awaitable_map.insert({co_id, [sptr_c](){
            sptr_c->resume();
        }});
        return std::move(g_cppt_co_c);
    });
}
