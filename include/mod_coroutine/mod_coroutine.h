#ifndef CPP_TOOLKIT_MOD_COROUTINE_H
#define CPP_TOOLKIT_MOD_COROUTINE_H

#include <functional>
#include <mod_common/utils.h>
#include <boost/context/continuation.hpp>

extern thread_local boost::context::continuation g_cppt_co_c;

void cppt_co_create0(std::function<void()> user_co);

template<typename Function, typename... Args>
void cppt_co_create(Function& f, Args... args)
{
    auto params = std::make_tuple(std::forward<Args>(args)...);
    auto user_co = [=](){
        call_with_variadic_arg(f, params);
    };
    cppt_co_create0(std::move(user_co));
}

unsigned int cppt_co_awaitable_create0(std::function<void()> user_co);

template<typename Function, typename... Args>
unsigned int cppt_co_awaitable_create(Function& f, Args... args)
{
    auto params = std::make_tuple(std::forward<Args>(args)...);
    auto user_co = [=](){
        call_with_variadic_arg(f, params);
    };
    return cppt_co_awaitable_create0(std::move(user_co));
}

void cppt_co_main_run();

void cppt_co_yield(std::function<void(std::function<void()>&&)> wrapped_extern_func);
void cppt_co_await(unsigned int co_id);

void cppt_co_add_c(boost::context::continuation&& c);

#endif //CPP_TOOLKIT_MOD_COROUTINE_H
