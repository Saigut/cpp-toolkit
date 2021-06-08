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
            : m_net_io_worker(other_worker)
    {}

    int netio_connect(const std::string addr_str, uint16_t port)
    {
        boost::system::error_code ec;
        boost::asio::ip::address addr = boost::asio::ip::make_address("127.0.0.1", ec);
        check_ec_ret_val(ec, -1, "make_address");
        tcp::endpoint endpoint(addr, 80);

        Work_NetTcpConnect work_connect(this);
        work_connect.m_endpoint = endpoint;
        expect_ret_val(0 == m_net_io_worker->add_work(&work_connect), -1);
        expect_ret_val(m_wp, -1);
        m_wp = m_wp.resume();
        m_socket_to_server = work_connect.m_socket_to_server;
        return 0;
    }

    int netio_write(char* str_buf, size_t str_len)
    {
        Work_NetTcpOut work_out(this);
        work_out.out_buf = boost::asio::mutable_buffer(str_buf, str_len);
        work_out.m_socket = m_socket_to_server;
        expect_ret_val(0 == m_net_io_worker->add_work(&work_out), -1);
        expect_ret_val(m_wp, -1);
        m_wp = m_wp.resume();
        return 0;
    }

    int netio_read(char* recv_buf, size_t buf_sz, size_t& recv_data_sz)
    {
        Work_NetTcpIn work_in(this);
        work_in.in_buf = boost::asio::mutable_buffer(recv_buf, buf_sz);
        work_in.m_socket = m_socket_to_server;
        expect_ret_val(0 == m_net_io_worker->add_work(&work_in), -1);
        expect_ret_val(m_wp, -1);
        m_wp = m_wp.resume();
        recv_data_sz = work_in.in_buf.size();
        return 0;
    }

    void do_work() override
    {
        // Connect
        expect_ret(0 == netio_connect("127.0.0.1", 80));

        for (unsigned i = 0; i < 100; i++) {
            char send_buf[4096];
            snprintf(send_buf, sizeof(send_buf), "GET / HTTP/1.1\r\n"
                                                 "Host: 127.0.0.1\r\n"
                                                 "Connection: keep-alive\r\n"
                                                 "User-Agent: curl/7.58.0\r\n"
                                                 "Accept: */*\r\n\r\n");
            // Send
            expect_ret(0 == netio_write(send_buf, std::strlen(send_buf)));

            // Recv
            char recv_buf[4096] = { 0 };
            size_t recv_data_sz = 0;
            expect_ret(0 == netio_read(recv_buf, sizeof(recv_buf), recv_data_sz));

            recv_buf[recv_data_sz < sizeof(recv_buf) ? recv_data_sz : sizeof(recv_buf) - 1] = 0;
//            log_info("received:\n%s!", recv_buf);
        }
        m_socket_to_server->close();
    };

private:
    Worker_NetIo* m_net_io_worker = nullptr;
    std::shared_ptr<tcp::socket> m_socket_to_server = nullptr;
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

    // Start other_worker
    std::thread other_worker_thr(worker_thread, &worker_netio);
    std::this_thread::sleep_for(std::chrono::seconds (1));

    // Start main worker
    std::thread main_worker_thr(main_worker_thread, &worker);


    // Add work to main worker
//    SomeThing a_work{&worker_netio};
//    worker.add_work(&a_work);
    std::vector<SomeThing> works;
    int i;
    int num = 500;
    for (i = 0; i < num; i++) {
        works.emplace_back(&worker_netio);
    }
    for (i = 0; i < num; i++) {
        worker.add_work(&(works.at(i)));
//        std::this_thread::sleep_for(std::chrono::microseconds(1000));
    }

    while (true)  {
        std::this_thread::sleep_for(std::chrono::milliseconds (10));
    }

    return 0;
}
