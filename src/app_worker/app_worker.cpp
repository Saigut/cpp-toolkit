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
    explicit SomeThing(Work* consignor_work)
            : Work(consignor_work)
    {}
    explicit SomeThing(Worker_NetIo* other_worker)
            : m_net_io_worker(other_worker)
    {}

    void do_work() override
    {
        // Connect
        WorkUtils::TcpSocketConnector_Asio tcp_socket(m_net_io_worker, this, m_wp);
        expect_ret(0 == tcp_socket.connect("127.0.0.1", 80));

        for (unsigned i = 0; i < 10; i++) {
            char send_buf[4096];
            snprintf(send_buf, sizeof(send_buf), "GET / HTTP/1.1\r\n"
                                                 "Host: 127.0.0.1\r\n"
                                                 "Connection: keep-alive\r\n"
                                                 "User-Agent: curl/7.58.0\r\n"
                                                 "Accept: */*\r\n\r\n");
            // Send
            expect_ret(0 == tcp_socket.write(send_buf, std::strlen(send_buf)));

            // Recv
            char recv_buf[4096] = { 0 };
            size_t recv_data_sz = 0;
            expect_ret(0 == tcp_socket.read(recv_buf, sizeof(recv_buf), recv_data_sz));

            recv_buf[recv_data_sz < sizeof(recv_buf) ? recv_data_sz : sizeof(recv_buf) - 1] = 0;
//            log_info("received:\n%s!", recv_buf);
        }
    };

private:
    Worker_NetIo* m_net_io_worker = nullptr;
};


void main_worker_thread(Worker* worker)
{
    worker->run();
}

void worker_thread(Worker_NetIo* worker)
{
    worker->run();
}

int app_worker(int argc, char** argv)
{
    Worker worker{};
    Worker_NetIo worker_net_io{};

    // Start main worker
    std::thread main_worker_thr(main_worker_thread, &worker);
    // Start net io worker
    std::thread other_worker_thr(worker_thread, &worker_net_io);

    // Add work to main worker
    worker_net_io.wait_worker_started();
    std::vector<SomeThing> works;
    int i;
    int num = 500;
    for (i = 0; i < num; i++) {
        works.emplace_back(&worker_net_io);
    }
    for (i = 0; i < num; i++) {
        worker.add_work(&(works.at(i)));
    }

    while (true)  {
        std::this_thread::sleep_for(std::chrono::milliseconds (100));
    }

    return 0;
}
