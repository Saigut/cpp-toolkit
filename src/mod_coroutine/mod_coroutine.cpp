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
    virtual void start_user_co();
protected:
    std::function<void()> m_user_co;
};

class cppt_co_wrapper_awaitable : public cppt_co_wrapper {
public:
    explicit cppt_co_wrapper_awaitable(std::function<void()> user_co, uint32_t co_id)
            : cppt_co_wrapper(std::move(user_co)), m_co_id(co_id) {}
    explicit cppt_co_wrapper_awaitable(context::continuation&& c, uint32_t co_id)
            : cppt_co_wrapper(std::move(c)), m_co_id(co_id) {}
    void start_user_co() override;
private:
    uint32_t m_co_id;
};

using cppt_co_queue_t = atomic_queue::AtomicQueue2<std::shared_ptr<cppt_co_wrapper>, 65535>;


// Values
thread_local context::continuation g_cppt_co_c;
thread_local std::map<uint32_t, std::function<void()>> g_co_awaitable_map;
thread_local std::queue<uint32_t> g_awaitable_id_queue;
cppt_co_queue_t g_co_exec_queue;


// Helpers
static void init_awaitable_id_queue()
{
    int i;
    for (i = 0; i < 65536; i++) {
        g_awaitable_id_queue.push(i);
    }
}
static void cppt_co_add_c(context::continuation&& c)
{
    g_co_exec_queue.push(std::make_shared<cppt_co_wrapper>(std::move(c)));
}


// Type implementation
void cppt_co_wrapper::start_user_co()
{
    context::callcc([&](context::continuation && c) {
        g_cppt_co_c = std::move(c);
        m_user_co();
        return std::move(g_cppt_co_c);
    });
}

void cppt_co_wrapper_awaitable::start_user_co()
{
    context::callcc([&, co_id(m_co_id)](context::continuation && c) {
        g_cppt_co_c = std::move(c);
        m_user_co();
        auto rst = g_co_awaitable_map.find(co_id);
        if (rst != g_co_awaitable_map.end()) {
            if (rst->second) {
                cppt_co_create(rst->second);
            }
        }
        g_co_awaitable_map.erase(rst);
        g_awaitable_id_queue.push(co_id);

        return std::move(g_cppt_co_c);
    });
}


// Interfaces
void cppt_co_create(std::function<void()> user_co)
{
    g_co_exec_queue.push(std::make_shared<cppt_co_wrapper>(std::move(user_co)));
}

unsigned int cppt_co_awaitable_create(std::function<void()> user_co)
{
    if (g_awaitable_id_queue.empty()) {
        log_error("No available await id to use!");
        return -1;
    }
    uint32_t id = g_awaitable_id_queue.front();
    g_awaitable_id_queue.pop();
    g_co_exec_queue.push(std::make_shared<cppt_co_wrapper_awaitable>(
            std::move(user_co), id));
    return id;
}

void cppt_co_main_run()
{
    init_awaitable_id_queue();
    while (true) {
        if (!g_co_exec_queue.was_empty()) {
            std::shared_ptr<cppt_co_wrapper> co = g_co_exec_queue.pop();
            if (co->m_c) {
                co->m_c.resume();
            } else {
                co->start_user_co();
            }
        } else {
            cppt_usleep(1);
        }
    }
}

void cppt_co_yield(std::function<void(std::function<void()>&&)> wrapped_extern_func)
{
    g_cppt_co_c = context::callcc([&](context::continuation && c) {
        auto sptr_c = std::make_shared<context::continuation>(std::move(c));
        wrapped_extern_func([sptr_c](){ cppt_co_add_c(std::move(*sptr_c)); });
        return std::move(g_cppt_co_c);
    });
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
