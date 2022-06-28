#ifndef CPP_TOOLKIT_MOD_COROUTINE_H
#define CPP_TOOLKIT_MOD_COROUTINE_H

#include <queue>
#include <memory>
#include <boost/context/continuation.hpp>

namespace context = boost::context;

extern thread_local context::continuation g_cppt_co_c;
#define cppt_co_yield() g_cppt_co_c = g_cppt_co_c.resume()

class cppt_co_wrapper {
public:
    explicit cppt_co_wrapper(std::function<void()> user_co)
            : m_user_co(std::move(user_co)) {}
    int exec();
private:
    std::function<void()> m_user_co;
    context::continuation m_c;
};
extern std::queue<std::shared_ptr<cppt_co_wrapper>> cppt_co_exec_queue;

void co_add_user_co(std::function<void()> user_co);
void co_run();

#endif //CPP_TOOLKIT_MOD_COROUTINE_H
