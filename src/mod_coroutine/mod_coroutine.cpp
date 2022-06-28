#include <mod_coroutine/mod_coroutine.h>

thread_local context::continuation g_cppt_co_c;
std::queue<std::shared_ptr<cppt_co_wrapper>> cppt_co_exec_queue;

static context::continuation co_helper_k(std::function<void()> user_co) {
    return context::callcc([&](context::continuation && c) {
        g_cppt_co_c = std::move(c);
        user_co();
        return std::move(g_cppt_co_c);
    });
}

int cppt_co_wrapper::exec() {
    if (!m_c) {
        m_c = co_helper_k(m_user_co);
    } else {
        m_c = m_c.resume();
    }
    if (!m_c) {
        return -1;
    }
    return 0;
}

void co_add_user_co(std::function<void()> user_co)
{
    cppt_co_exec_queue.push(std::make_shared<cppt_co_wrapper>(std::move(user_co)));
}

void co_run()
{
    while (!cppt_co_exec_queue.empty()) {
        std::shared_ptr<cppt_co_wrapper> co = cppt_co_exec_queue.front();
        cppt_co_exec_queue.pop();
        if (0 == co->exec()) {
            cppt_co_exec_queue.push(co);
        }
    }
}
