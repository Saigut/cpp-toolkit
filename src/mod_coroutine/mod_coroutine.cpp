#include <mod_coroutine/mod_coroutine.h>

#include <queue>
#include <map>
#include <memory>
#include <boost/context/continuation.hpp>
#include <mod_atomic_queue/atomic_queue.h>
#include <mod_common/utils.h>


// Types
namespace context = boost::context;

class cppt_co_wrapper {
public:
    explicit cppt_co_wrapper(std::function<void()> user_co)
            : m_user_co(std::move(user_co)) {}
    explicit cppt_co_wrapper(context::continuation&& c)
            : m_c(std::move(c)) {}
    context::continuation m_c;
    virtual context::continuation start_user_co();
    void call_after_pause_handler();
    void set_after_pause_handler(
            const std::function<void(std::function<void()>&&)>& f);
//protected:
    std::function<void()> m_user_co;
    std::function<void(std::function<void()>&&)> m_after_pause_handler;
};

class cppt_co_wrapper_awaitable : public cppt_co_wrapper {
public:
    explicit cppt_co_wrapper_awaitable(std::function<void()> user_co, uint32_t co_id)
            : cppt_co_wrapper(std::move(user_co)), m_co_id(co_id) {}
    explicit cppt_co_wrapper_awaitable(context::continuation&& c, uint32_t co_id)
            : cppt_co_wrapper(std::move(c)), m_co_id(co_id) {}
    context::continuation start_user_co() override;
private:
    uint32_t m_co_id;
};

using cppt_co_queue_t = atomic_queue::AtomicQueue2<cppt_co_wrapper*, 65535>;


// Values
thread_local context::continuation g_cppt_co_c;
thread_local std::map<uint32_t, std::function<void()>> g_co_awaitable_map;
thread_local std::queue<uint32_t> g_awaitable_id_queue;
thread_local cppt_co_wrapper* g_cur_co;
cppt_co_queue_t g_co_exec_queue;


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
    auto new_co = new cppt_co_wrapper(std::move(c));
    if (!g_co_exec_queue.try_push(new_co)) {
        log_error("g_co_exec_queue full!");
        delete new_co;
    }
}
static void cppt_co_add_sptr(cppt_co_wrapper* wrapper)
{
    if (!g_co_exec_queue.try_push(wrapper)) {
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

void cppt_co_wrapper::call_after_pause_handler()
{
    if (m_after_pause_handler) {
        m_after_pause_handler([cur_co(g_cur_co)](){ cppt_co_add_sptr(cur_co); });
        m_after_pause_handler = nullptr;
    }
}

void cppt_co_wrapper::set_after_pause_handler(
        const std::function<void(std::function<void()>&&)>& f)
{
    m_after_pause_handler = f;
}

// Interfaces
void cppt_co_create0(std::function<void()> user_co)
{
    g_co_exec_queue.push(new cppt_co_wrapper(std::move(user_co)));
}

unsigned int cppt_co_awaitable_create0(std::function<void()> user_co)
{
    if (g_awaitable_id_queue.empty()) {
        log_error("No available await id to use!");
        return -1;
    }
    uint32_t id = g_awaitable_id_queue.front();
    g_awaitable_id_queue.pop();
    g_co_exec_queue.push(new cppt_co_wrapper_awaitable(
            std::move(user_co), id));
    return id;
}

void cppt_co_main_run()
{
    init_awaitable_id_queue();
    while (true) {
        if (!g_co_exec_queue.was_empty()) {
            g_cur_co = g_co_exec_queue.pop();
            if (g_cur_co->m_c) {
                g_cur_co->m_c = std::move(g_cur_co->m_c.resume());
            } else {
                g_cur_co->m_c = std::move(g_cur_co->start_user_co());
            }
//            g_cur_co->call_after_pause_handler();
            if (g_cur_co->m_after_pause_handler) {
                g_cur_co->m_after_pause_handler([cur_co(g_cur_co)](){ cppt_co_add_sptr(cur_co); });
                g_cur_co->m_after_pause_handler = nullptr;
            } else {
                free(g_cur_co);
                g_cur_co = nullptr;
            }
        } else {
            cppt_nanosleep(1);
        }
    }
}

static void set_cur_c_call_after_pause_handler(const std::function<void(std::function<void()>&&)>& f)
{
    g_cur_co->set_after_pause_handler(f);
}

void cppt_co_yield(const std::function<void(std::function<void()>&&)>& wrapped_extern_func)
{
    set_cur_c_call_after_pause_handler(wrapped_extern_func);
    g_cppt_co_c = g_cppt_co_c.resume();
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
