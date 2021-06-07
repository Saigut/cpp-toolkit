#include <app_worker/app_worker.h>
#include <mod_worker/mod_work.h>
#include <mod_worker/mod_worker_netio.h>

#include <mod_common/expect.h>

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
    explicit SomeThing(Work* consignor)
            : Work(consignor)
    {}
    explicit SomeThing(Worker_NetIo* other_worker)
            : m_other_worker(other_worker)
    {}
    void do_work() override
    {
        boost::system::error_code ec;

        boost::asio::ip::address addr = boost::asio::ip::make_address("127.0.0.1", ec);
        check_ec_ret(ec, "make_address");
        tcp::endpoint endpoint(addr, 80);

        Work_NetTcpConnect work_connect(this);
        work_connect.m_endpoint = endpoint;
        expect_ret(0 == m_other_worker->add_work(&work_connect));
        expect_ret(m_wp);
        m_wp = m_wp.resume();

        char send_buf[4096];
        snprintf(send_buf, sizeof(send_buf), "GET / HTTP/1.1\r\n\r\n");
        Work_NetTcpOut work_out(this);
        work_out.out_buf = boost::asio::mutable_buffer(send_buf, std::strlen(send_buf));
        work_out.m_socket = work_connect.m_socket_to_server;
        expect_ret(0 == m_other_worker->add_work(&work_out));
        expect_ret(m_wp);
        m_wp = m_wp.resume();

        char recv_buf[4096];
        Work_NetTcpIn work_in(this);
        work_in.in_buf = boost::asio::mutable_buffer(recv_buf, sizeof(recv_buf));
        work_in.m_socket = work_connect.m_socket_to_server;
        expect_ret(0 == m_other_worker->add_work(&work_in));
        expect_ret(m_wp);
        m_wp = m_wp.resume();
        work_connect.m_socket_to_server->close();

        recv_buf[work_in.in_buf.size()] = 0;
//        log_info("received:\n%s!", recv_buf);
    };
    Worker_NetIo* m_other_worker = nullptr;
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
    Worker_NetIo worker_netio{};

    // Add work to worker1
    std::vector<SomeThing> works;

    // Start worker2
    std::thread other_worker_thr(worker_thread, &worker_netio);
    std::this_thread::sleep_for(std::chrono::seconds (1));

    // Start main worker
    std::thread main_worker_thr(main_worker_thread, &worker);

//    SomeThing a_work{&worker_netio};
//    worker.add_work(&a_work);

    int num = 300;
    for (int j = 0; j < 100; j++) {
        for (int i = 0; i < num; i++) {
            works.emplace_back(&worker_netio);
        }
        for (int i = 0; i < num; i++) {
            worker.add_work(&(works.at(j*num + i)));
        }
        std::this_thread::sleep_for(std::chrono::milliseconds (100));
    }

    while (true)  {
        std::this_thread::sleep_for(std::chrono::milliseconds (10));
    }

    return 0;
}
