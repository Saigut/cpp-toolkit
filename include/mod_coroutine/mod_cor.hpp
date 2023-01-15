#ifndef CPP_TOOLKIT_MOD_COR_HPP
#define CPP_TOOLKIT_MOD_COR_HPP

#include "mod_cor_impl.hpp"

namespace cppt {
    // cppt:
    // cor_sp
    // cor_sp->join()
    // cor_create()
    // cor_yield()
    // cor_run()

    using cor_sp = cppt_impl::cor_sp;

    template<typename Function, typename... Args>
    cor_sp cor_create(Function& f, Args... args)
    {
        auto params = std::make_tuple(std::forward<Args>(args)...);
        auto user_co = [=](){
            cppt_impl::co_call_with_variadic_arg(f, params);
        };
        return cppt_impl::cppt_co_create0(std::move(user_co));
    }

    template<typename Function, typename... Args>
    cor_sp cor_create(Function&& f, Args... args)
    {
        auto params = std::make_tuple(std::forward<Args>(args)...);
        auto user_co = [=](){
            cppt_impl::co_call_with_variadic_arg(f, params);
        };
        return cppt_impl::cppt_co_create0(std::move(user_co));
    }

    // ret: 0, ok; -1 coroutine error
    int cor_yield(
            const std::function<void(std::function<void()>&& resume_f)>& wrapped_extern_func);

    // ret: 0, ok; 1 timeout
    int cor_yield(
            const std::function<void(std::function<void()>&& resume_f)>& wrapped_extern_func,
            unsigned int timeout_ms,
            const std::function<void()>& f_cancel_operation);

    void cor_run();
}

#endif //CPP_TOOLKIT_MOD_COR_HPP
