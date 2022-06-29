#include <mod_coroutine/mod_coroutine.h>

#include <queue>
#include <memory>
#include <boost/context/continuation.hpp>
#include <mod_atomic_queue/atomic_queue.h>


// Types
namespace context = boost::context;

class cppt_co_wrapper {
public:
    explicit cppt_co_wrapper(std::function<void()> user_co)
            : m_user_co(std::move(user_co)) {}
    explicit cppt_co_wrapper(context::continuation&& c)
            : m_c(std::move(c)) {}
    context::continuation m_c;
    void start_user_co();
private:
    std::function<void()> m_user_co;
};

using cppt_co_queue_t = atomic_queue::AtomicQueue2<std::shared_ptr<cppt_co_wrapper>, 65535>;


// Values
thread_local context::continuation g_cppt_co_c;
cppt_co_queue_t cppt_co_exec_queue;


// Helpers
static void cppt_co_add_c(context::continuation&& c)
{
    cppt_co_exec_queue.push(std::make_shared<cppt_co_wrapper>(std::move(c)));
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


// Interfaces
void cppt_co_create(std::function<void()> user_co)
{
    cppt_co_exec_queue.push(std::make_shared<cppt_co_wrapper>(std::move(user_co)));
}

void cppt_co_main_run()
{
    while (true) {
        if (!cppt_co_exec_queue.was_empty()) {
            std::shared_ptr<cppt_co_wrapper> co = cppt_co_exec_queue.pop();
            if (co->m_c) {
                co->m_c.resume();
            } else {
                co->start_user_co();
            }
        } else {
            usleep(1);
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
