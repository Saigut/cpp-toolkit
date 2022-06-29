#ifndef CPP_TOOLKIT_MOD_COROUTINE_H
#define CPP_TOOLKIT_MOD_COROUTINE_H

#include <functional>

void cppt_co_create(std::function<void()> user_co);
void cppt_co_main_run();

void cppt_co_yield(std::function<void(std::function<void()>&&)> wrapped_extern_func);

#endif //CPP_TOOLKIT_MOD_COROUTINE_H
