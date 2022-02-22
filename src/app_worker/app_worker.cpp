#include <app_worker/app_worker.h>
#include <mod_worker/mod_work.h>
#include <mod_worker/mod_worker_netio.h>

#include <mod_common/expect.h>
#include <mod_worker/mod_workutils.h>

/*#include <cstdlib>

#include <boost/context/continuation.hpp>
#include <boost/context/fiber.hpp>

namespace ctx = boost::context;

ctx::continuation foo( ctx::continuation && c)
{
    do {
        std::cout << "foo\n";
    } while ( ( c = c.resume() ) );
    std::cout << "before return\n";
    return std::move( c);
}

int ctx_main() {
    ctx::continuation c = ctx::callcc( foo);
    do {
        std::cout << "bar\n";
    } while ( ( c = c.resume() ) );
    std::cout << "main: done" << std::endl;
    return EXIT_SUCCESS;
}

ctx::fiber bar( ctx::fiber && f)
{
    do {
        std::cout << "bar\n";
        f = std::move( f).resume();
    } while ( f);
    return std::move( f);
}

int fb_main()
{
    ctx::fiber f{ bar };
    do {
        std::cout << "foo\n";
        f = std::move( f).resume();
    } while ( f);
    std::cout << "main: done" << std::endl;
    return EXIT_SUCCESS;
}*/


/*void worker2_thread(Worker* worker)
{
    worker->run();
}

int test_worker()
{
    Worker worker1{};
    Worker_Net worker2{};

    // Add work to worker1
    Work_ImSend thing{&worker2};
    worker1.add_work(&thing);

    // Start worker2
    std::thread other_thread(worker2_thread, &worker2);
    std::this_thread::sleep_for(std::chrono::seconds (1));

    // Start worker1
    worker1.run();

    return 0;
}*/


class SomeThing : public Work {
public:
    explicit SomeThing(io_context& io_ctx)
            : m_io_ctx(io_ctx)
    {}

    void do_work() override
    {
        std::shared_ptr<Work> this_obj = shared_from_this();
        WorkUtils::WorkCoCbs co_cbs{[this_obj]() { this_obj->m_wp.wp_yield(0); },
                                    [this_obj]() { this_obj->add_self_back_to_main_worker(nullptr); }};
        WorkUtils::TcpSocketBuilder_Asio skt_builder(m_io_ctx, co_cbs);
        // Connect
        auto skt = skt_builder.connect("127.0.0.1", 80);
        expect_ret(skt);

        unsigned i;
        for (i = 0; i < 10; i++) {
            char send_buf[4096];
            snprintf(send_buf, sizeof(send_buf), "GET / HTTP/1.1\r\n"
                                                 "Host: 127.0.0.1\r\n"
                                                 "Connection: keep-alive\r\n"
                                                 "User-Agent: curl/7.58.0\r\n"
                                                 "Accept: */*\r\n\r\n");
            // Send
            expect_goto(skt->write((uint8_t*)send_buf, std::strlen(send_buf)), func_return);

            // Recv
            char recv_buf[4096] = { 0 };
            int ret_recv = skt->read_some((uint8_t*)recv_buf, sizeof(recv_buf));
            expect_goto(ret_recv > 0, func_return);

            recv_buf[ret_recv < sizeof(recv_buf) ? ret_recv : sizeof(recv_buf) - 1] = 0;
            log_info("received:\n%s!", recv_buf);
        }
        func_return:
        printf("done %u req\n", i);
    };

private:
    io_context& m_io_ctx;
};


static void main_worker_thread(Worker* worker)
{
    worker->run();
}

static void worker_thread(io_context* io_ctx)
{
    boost::asio::io_context::work io_work(*io_ctx);
    io_ctx->run();
    log_info("Asio io context quit!!");
}

int app_worker(int argc, char** argv)
{
    Worker worker{};
    io_context io_ctx;

    // Start main worker
    std::thread main_worker_thr(main_worker_thread, &worker);
    // Start net io worker
    std::thread other_worker_thr(worker_thread, &io_ctx);

    // Add work to main worker
    std::vector<std::shared_ptr<WorkWrap>> works;
    int i;
    int num = 1;
//    for (i = 0; i < num; i++) {
//        works.emplace_back(&worker_net_io);
//    }
    for (i = 0; i < num; i++) {
        worker.add_work(new WorkWrap(std::make_shared<SomeThing>(io_ctx), nullptr));
    }

    while (true)  {
        std::this_thread::sleep_for(std::chrono::milliseconds (100));
    }

    return 0;
}
