#include <app_perf_test/app_perf_test.h>

#include <functional>
#include <stdint.h>
#include <mod_common/log.h>
#include <mod_common/utils.h>

struct log_record {
    uint64_t t1;
    uint64_t t2;
    uint64_t t3;
    uint64_t t4;
    uint64_t t5;
    uint64_t t6;
};

void print_log_record(log_record& log)
{
    log_info("t1: %lluus", log.t1);
    log_info("t2: %lluus", log.t2);
//    log_info("t3: %lluus", log.t3);
//    log_info("t4: %lluus", log.t4);
//    log_info("t5: %lluus", log.t5);
//    log_info("t6: %lluus", log.t6);
}

int g_v = 0;

template<typename Functor>
void lmd_func_val_rref_templ(Functor&& f)
{
    std::forward<Functor>(f)();;
}

void lmd_func_val_rref(std::function<void()>&& f)
{
    f();
}

template<typename Functor>
void lmd_func_val_ref(Functor& f)
{
    f();
}

void lmd_func_val(std::function<void()> f)
{
    f();
}

void c_func()
{
    g_v++;
}
typedef void(*c_f_p_t)();
void lmd_func_c(c_f_p_t f)
{
    f();
}


void test_lambda()
{
    log_record logs;
    const int num = 1000;
    int i;
    int& local_g_v = g_v;

    log_info("c func pointer");
    g_v = 0;
    logs.t1 = util_now_ts_us();
    for (i = 0; i < num; i++) {
        lmd_func_c(c_func);
    }
    logs.t2 = util_now_ts_us();
    print_log_record(logs);

    log_info("c copy lambda");
    g_v = 0;
    logs.t1 = util_now_ts_us();
    for (i = 0; i < num; i++) {
        auto f = [&local_g_v]() { local_g_v++; };
//        lmd_func_val([&local_g_v]() { local_g_v++; });
        lmd_func_val(f);
    }
    logs.t2 = util_now_ts_us();
    print_log_record(logs);

    log_info("c reference templ of lambda");
    g_v = 0;
    logs.t1 = util_now_ts_us();
    for (i = 0; i < num; i++) {
        auto f = [&local_g_v]() { local_g_v++; };
        lmd_func_val_ref(f);
    }
    logs.t2 = util_now_ts_us();
    print_log_record(logs);

    log_info("c rvalue of lambda");
    g_v = 0;
    logs.t1 = util_now_ts_us();
    for (i = 0; i < num; i++) {
        auto f = [&local_g_v]() { local_g_v++; };
        lmd_func_val_rref(std::move(f));
    }
    logs.t2 = util_now_ts_us();
    print_log_record(logs);

    log_info("c rvalue templ of lambda");
    g_v = 0;
    logs.t1 = util_now_ts_us();
    for (i = 0; i < num; i++) {
        auto f = [&local_g_v]() { local_g_v++; };
        lmd_func_val_rref_templ(std::move(f));
    }
    logs.t2 = util_now_ts_us();
    print_log_record(logs);
}

int app_perf_test(int argc, char** argv)
{
    test_lambda();
    return 0;
}
