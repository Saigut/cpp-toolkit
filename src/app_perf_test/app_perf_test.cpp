#include <app_perf_test/app_perf_test.h>

#include <functional>
#include <thread>
#include <tuple>
#include <stdint.h>
#include <boost/context/continuation.hpp>
#include <boost/context/fiber.hpp>
#include <mod_common/log.h>
#include <mod_common/utils.h>
#include <mod_coroutine/mod_coroutine.h>
#include "app_perf_test_internal.h"
#include "clock.hpp"

struct log_record {
    uint64_t t1;
    uint64_t t2;
    uint64_t t3;
    uint64_t t4;
    uint64_t t5;
    uint64_t t6;
};

static void print_log_record(log_record& log)
{
    log_info("[t1,t2]: %" PRIu64 "us", log.t2 - log.t1);
//    log_info("[t2,t3]: %lluus", log.t3 - log.t2);
//    log_info("t3: %lluus", log.t3);
//    log_info("t4: %lluus", log.t4);
//    log_info("t5: %lluus", log.t5);
//    log_info("t6: %lluus", log.t6);
}

int g_v = 0;

template<typename Functor>
static void lmd_func_val_rref_templ(Functor&& f)
{
    std::forward<Functor>(f)();;
}

static void lmd_func_val_rref(std::function<void()>&& f)
{
    f();
}

template<typename Functor>
static void lmd_func_val_ref(Functor& f)
{
    f();
}

static void lmd_func_val(std::function<void()> f)
{
    f();
}

static void c_func()
{
    g_v++;
}
typedef void(*c_f_p_t)();
static void lmd_func_c(c_f_p_t f)
{
    f();
}


static void test_lambda()
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

    log_info("c copy lambda 2");
    g_v = 0;
    logs.t1 = util_now_ts_us();
    auto f2 = [&local_g_v]() { local_g_v++; };
    for (i = 0; i < num; i++) {
//        lmd_func_val([&local_g_v]() { local_g_v++; });
        lmd_func_val(f2);
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

static void fun2( std::function<void()> callback ) {
    (callback)();
}

static void fun1(int n) {
    if(n <= 0) return;
    printf("stack address = %p, ", &n);

    fun2([n]() {
        printf("capture address = %p\n", &n);
        fun1(n - 1);
    });
}

static int test2() {
    fun1(200);
    return 0;
}

std::function<void()> g_func_lambda;
template<typename Function, typename... Args>
static void save_func_lambda(Function& f, Args... args)
{
    auto params = std::make_tuple(std::forward<Args>(args)...);
    auto user_co = [=](){
        call_with_variadic_arg(f, params);
    };
    g_func_lambda = std::move(user_co);
}

static void call_func_lambda()
{
    g_func_lambda();
}

typedef void* func_cast_t;

func_cast_t g_f_cast;

template<size_t... Indexes> struct _Index_tuple { };

#ifdef __has_builtin
# if __has_builtin(__make_integer_seq)
#  define _GLIBCXX_USE_MAKE_INTEGER_SEQ 1
# endif
#endif

// Builds an _Index_tuple<0, 1, 2, ..., _Num-1>.
template<size_t Num>
struct Build_index_tuple
{
#if _GLIBCXX_USE_MAKE_INTEGER_SEQ
    template<typename, size_t... Indexes>
        using _IdxTuple = _Index_tuple<Indexes...>;

      using __type = __make_integer_seq<_IdxTuple, size_t, Num>;
#else
    using __type = _Index_tuple<__integer_pack(Num)...>;
#endif
};

struct State
{
    virtual ~State() = default;
    virtual void _M_run() = 0;
};
using State_ptr = std::unique_ptr<State>;

template<typename Tuple>
struct Invoker
{
    Tuple M_t;

    template<typename>
    struct _result;
    template<typename Fn, typename... Args>
    struct _result<std::tuple<Fn, Args...>>
            : std::invoke_result<Fn, Args...>
    { };

    template<size_t... Ind>
    typename _result<Tuple>::type
    M_invoke(_Index_tuple<Ind...>)
    { return std::invoke(std::get<Ind>(std::move(M_t))...); }

    typename _result<Tuple>::type
    operator()()
    {
        using Indices
        = typename Build_index_tuple<std::tuple_size<Tuple>::value>::__type;
        return M_invoke(Indices());
    }
};

template<typename... Tp>
using decayed_tuple = std::tuple<typename std::decay<Tp>::type...>;

template<typename _Callable, typename... _Args>
static Invoker<decayed_tuple<_Callable, _Args...>>
make_invoker(_Callable&& _callable, _Args&&... _args)
{
    return { decayed_tuple<_Callable, _Args...>{
        std::forward<_Callable>(_callable), std::forward<_Args>(_args)...
    } };
}

template<typename Callable>
struct State_impl : public State
{
    Callable M_func;

    State_impl(Callable&& _f) : M_func(std::forward<Callable>(_f))
    { }

    void
    _M_run() { M_func(); }
};

template<typename Callable>
static State_ptr S_make_state(Callable&& _f)
{
    using _co_Impl = State_impl<Callable>;
    return State_ptr{new _co_Impl{std::forward<Callable>(_f)}};
}

static void save_func_cast0(State_ptr state_ptr)
{
    g_f_cast = state_ptr.get();
//    log_info("g_f_cast: %p", g_f_cast);
    state_ptr.release();
}
template<typename Callable, typename... Args>
void save_func_cast(Callable&& _f, Args&&... _args)
{
    save_func_cast0(S_make_state(
            make_invoker(std::forward<Callable>(_f),
                         std::forward<Args>(_args)...)));
}

static void call_func_cast()
{
//    log_info("g_f_cast: %p", g_f_cast);
    State_ptr _t{ static_cast<State*>(g_f_cast) };
    _t->_M_run();
}

static void func_for_save(int n1, int n2)
{
    g_v = 0;
    g_v += n1;
    g_v *= n2;
}

static void test_save_func()
{
    log_record logs;
    const int num = 1000;
    int i;

    log_info("save func to lambda");
    logs.t1 = util_now_ts_us();
    for (i = 0; i < num; i++) {
        save_func_lambda(func_for_save, 2, 3);
        call_func_lambda();
    }
    logs.t2 = util_now_ts_us();
    print_log_record(logs);

    log_info("save func to to raw pointer");
    logs.t1 = util_now_ts_us();
    for (i = 0; i < num; i++) {
        save_func_cast(func_for_save, 2, 3);
        call_func_cast();
    }
    logs.t2 = util_now_ts_us();
    print_log_record(logs);
}

static void print_log_record_sptr(log_record& log)
{
    log_info("[t1,t2]: %" PRIu64 "us", log.t2 - log.t1);
    log_info("[t2,t3]: %" PRIu64 "us", log.t3 - log.t2);
//    log_info("t3: %lluus", log.t3);
//    log_info("t4: %lluus", log.t4);
//    log_info("t5: %lluus", log.t5);
//    log_info("t6: %lluus", log.t6);
}
class test_share_ptr_t {
public:
    test_share_ptr_t(int _x, int _y) : x(_x), y(_x) {}
    int total = 0;
    int x;
    int y;
    void exec(int _x, int _y)
    {
        total = (_x, _y);
    }
};

int g_x = 0;
int g_y = 0;

void use_sptr()
{
    auto use_sptr = std::make_shared<test_share_ptr_t>(g_x, g_y);
    use_sptr->exec(g_x, g_y);
    g_x++;
    g_y++;
}

void use_unique_sptr()
{
    auto use_sptr = std::make_unique<test_share_ptr_t>(g_x, g_y);
    use_sptr->exec(g_x, g_y);
    g_x++;
    g_y++;
}

void use_unique_sptr_new_delete()
{
    auto p =new test_share_ptr_t(g_x, g_y);
    p->exec(g_x, g_y);
    g_x++;
    g_y++;
    delete p;
}

void use_unique_sptr_malloc_free()
{
    auto p = (test_share_ptr_t*)malloc(sizeof(test_share_ptr_t));
    p->exec(g_x, g_y);
    g_x++;
    g_y++;
    free(p);
}

void no_sptr_pointer(test_share_ptr_t *sptr)
{
    sptr->exec(g_x, g_y);
    g_x++;
    g_y++;
}

void no_sptr_ref(test_share_ptr_t &sptr)
{
    sptr.exec(g_x, g_y);
    g_x++;
    g_y++;
}

void just_exec()
{
    int total = 0;
    total = (g_x, g_y);
    g_x++;
    g_y++;
}

void test_shared_ptr()
{
    log_record logs;
    const int num = 1000;
    int i;

    log_info("test shared ptr");
    logs.t1 = util_now_ts_us();
    for (i = 0; i < num; i++) {
        use_sptr();
    }
    logs.t2 = util_now_ts_us();
    print_log_record(logs);

    log_info("test unique ptr");
    logs.t1 = util_now_ts_us();
    for (i = 0; i < num; i++) {
        use_unique_sptr();
    }
    logs.t2 = util_now_ts_us();
    print_log_record(logs);

    log_info("test new/delete");
    logs.t1 = util_now_ts_us();
    for (i = 0; i < num; i++) {
        use_unique_sptr_new_delete();
    }
    logs.t2 = util_now_ts_us();
    print_log_record(logs);

    log_info("test malloc/free");
    logs.t1 = util_now_ts_us();
    for (i = 0; i < num; i++) {
        use_unique_sptr_malloc_free();
    }
    logs.t2 = util_now_ts_us();
    print_log_record(logs);

    log_info("test no shared ptr, pointer");
    test_share_ptr_t sptr_p{1, 2};
    logs.t1 = util_now_ts_us();
    for (i = 0; i < num; i++) {
        no_sptr_pointer(&sptr_p);
    }
    logs.t2 = util_now_ts_us();
    print_log_record(logs);

    log_info("test no shared ptr, ref");
    test_share_ptr_t sptr{1, 2};
    logs.t1 = util_now_ts_us();
    for (i = 0; i < num; i++) {
        no_sptr_ref(sptr);
    }
    logs.t2 = util_now_ts_us();
    print_log_record(logs);
}

#define get_ns_diff(a, b) (a) > (b) ? ((a) - (b)) : 99999999

static void print_log_record_ns(log_record& log)
{
    log_info("[t1,t2]: %luns", get_ns_diff(log.t2, log.t1));
    log_info("[t2,t3]: %luns", get_ns_diff(log.t3, log.t2));
    log_info("[t3,t4]: %luns", get_ns_diff(log.t4, log.t3));
    log_info("[t4,t5]: %luns", get_ns_diff(log.t5, log.t4));
//    log_info("[t5,t6]: %lluns", log.t5);
//    log_info("t4: %lluns", log.t4);
//    log_info("t5: %lluns", log.t5);
}

void cppt_co0()
{
    namespace context = boost::context;
    log_record logs;
    int i;
    const int switch_num = 100000;

    log_info("test cppt coroutine just switch 1 time");
    for (i = 0; i < 1; i++) {
        logs.t1 = util_now_ts_ns();
        g_cppt_co_c = context::callcc([&logs](context::continuation && c) {
            logs.t2 = util_now_ts_ns();
            cppt_co_add_c(std::move(c));
            logs.t3 = util_now_ts_ns();
            return std::move(g_cppt_co_c);
        });
        logs.t4 = util_now_ts_ns();
        logs.t5 = util_now_ts_ns();
    }
    print_log_record_ns(logs);

    log_info("test cppt coroutine 1 1time");
    for (i = 0; i < 1; i++) {
        logs.t1 = util_now_ts_ns();
        auto wrap_func = [&logs](std::function<void()>&& co_cb) {
            logs.t3 = util_now_ts_ns();
            co_cb();
            logs.t4 = util_now_ts_ns();
        };
        logs.t2 = util_now_ts_ns();
        cppt_co_yield(wrap_func);
        logs.t5 = util_now_ts_ns();
    }
    print_log_record_ns(logs);

    log_info("test cppt coroutine just switch");
    logs.t1 = util_now_ts_us();
    for (i = 0; i < switch_num; i++) {
        g_cppt_co_c = context::callcc([&logs](context::continuation && c) {
            cppt_co_add_c(std::move(c));
            return std::move(g_cppt_co_c);
        });
    }
    logs.t2 = util_now_ts_us();
    print_log_record(logs);

    log_info("test cppt coroutine 1");
    logs.t1 = util_now_ts_us();
    for (i = 0; i < switch_num; i++) {
        auto wrap_func = [&logs](std::function<void()>&& co_cb) {
            co_cb();
        };
        cppt_co_yield(wrap_func);
    }
    logs.t2 = util_now_ts_us();
    print_log_record(logs);

    log_info("test cppt coroutine 2");
    logs.t1 = util_now_ts_us();
    auto wrap_func = [&](std::function<void()>&& co_cb) {
        co_cb();
    };
    for (i = 0; i < switch_num; i++) {
        cppt_co_yield(wrap_func);
    }
    logs.t2 = util_now_ts_us();
    print_log_record(logs);

    log_info("test cppt coroutine 3");
    logs.t1 = util_now_ts_us();
    auto wrap_func3 = [](std::function<void()>&& co_cb) {
        co_cb();
    };
    for (i = 0; i < switch_num; i++) {
        cppt_co_yield(wrap_func3);
    }
    logs.t2 = util_now_ts_us();
    print_log_record(logs);
}

static void test_cppt_co()
{
    cppt_co_create(cppt_co0);
    cppt_co_main_run();
}

template< std::size_t Max, std::size_t Default, std::size_t Min >
class simple_stack_allocator
{
public:
    static std::size_t maximum_stacksize()
    { return Max; }

    static std::size_t default_stacksize()
    { return Default; }

    static std::size_t minimum_stacksize()
    { return Min; }

    void * allocate( std::size_t size) const
    {
        BOOST_ASSERT( minimum_stacksize() <= size);
        BOOST_ASSERT( maximum_stacksize() >= size);

        void * limit = std::malloc( size);
        if ( ! limit) throw std::bad_alloc();

        return static_cast< char * >( limit) + size;
    }

    void deallocate( void * vp, std::size_t size) const
    {
        BOOST_ASSERT( vp);
        BOOST_ASSERT( minimum_stacksize() <= size);
        BOOST_ASSERT( maximum_stacksize() >= size);

        void * limit = static_cast< char * >( vp) - size;
        std::free( limit);
    }
};

typedef simple_stack_allocator<
        8 * 1024 * 1024, 64 * 1024, 8 * 1024
>                                       stack_allocator;

static void test_boost_context()
{
    namespace context = boost::context;
    log_record logs;
    int i;
    const int switch_num = 100000;

    log_info("test boost callcc");
    context::continuation source = context::callcc(
            [](context::continuation && sink){
                for(;;){
                    sink=sink.resume();
                }
                return std::move(sink);
            });
    logs.t1 = util_now_ts_us();
    for (i = 0; i < switch_num; i++) {
        source=source.resume();
    }
    logs.t2 = util_now_ts_us();
    print_log_record(logs);

    log_info("test boost callcc init");
    {
        logs.t1 = util_now_ts_us();
        for (i = 0; i < switch_num; i++) {
            context::continuation source2 = context::callcc(
                    [&](context::continuation && sink){return std::move(sink);});
        }
        logs.t2 = util_now_ts_us();
        print_log_record(logs);
    }

    log_info("test boost fiber");
    context::fiber fsource{[](context::fiber&& sink){
        for(;;){
            sink=std::move(sink).resume();
        }
        return std::move(sink);
    }};
    logs.t1 = util_now_ts_us();
    for (i = 0; i < switch_num; i++) {
        fsource=std::move(fsource).resume();
    }
    logs.t2 = util_now_ts_us();
    print_log_record(logs);

    log_info("test boost fiber init");
    logs.t1 = util_now_ts_us();
    for (i = 0; i < switch_num; i++) {
        context::fiber fsource2{[](context::fiber&& sink){
            return std::move(sink);
        }};
    }
    logs.t2 = util_now_ts_us();
    print_log_record(logs);

    log_info("test boost fcontext");
    {
        stack_allocator stack_alloc;
        boost::context::detail::fcontext_t ctx = boost::context::detail::make_fcontext(
                stack_alloc.allocate(stack_allocator::default_stacksize()),
                stack_allocator::default_stacksize(),
                [](boost::context::detail::transfer_t t_) {
                    boost::context::detail::transfer_t t = t_;
                    while (true) {
                        t = boost::context::detail::jump_fcontext(t.fctx, 0);
                    }
                });
        // cache warum-up
        boost::context::detail::transfer_t t = boost::context::detail::jump_fcontext( ctx, 0);
        logs.t1 = util_now_ts_us();
        for (i = 0; i < switch_num; i++) {
            t = boost::context::detail::jump_fcontext( t.fctx, 0);
        }
        logs.t2 = util_now_ts_us();
        print_log_record(logs);
    }

    log_info("test boost fcontext init");
    {
        logs.t1 = util_now_ts_us();
        for (i = 0; i < switch_num; i++) {
            stack_allocator stack_alloc2;
            boost::context::detail::fcontext_t ctx2 = boost::context::detail::make_fcontext(
                    stack_alloc2.allocate(stack_allocator::default_stacksize()),
                    stack_allocator::default_stacksize(),
                    [](boost::context::detail::transfer_t t_) {
                        boost::context::detail::transfer_t t = t_;
                        t = boost::context::detail::jump_fcontext(t.fctx, 0);
                    });
            // cache warum-up
            boost::context::detail::transfer_t t = boost::context::detail::jump_fcontext( ctx2, 0);
        }
        logs.t2 = util_now_ts_us();
        print_log_record(logs);
    }
}

int app_perf_test(int argc, char** argv)
{
//    test_lambda();
//    test2();
//    test_save_func();
//    test_shared_ptr();
    test_boost_context();
    perf_callcc(argc, argv);
    perf_fiber(argc, argv);
    perf_fcontext(argc, argv);
    test_cppt_co();
    return 0;
}
