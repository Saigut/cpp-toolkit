#ifndef CPP_TOOLKIT_MOD_WORKER_NETIO_H
#define CPP_TOOLKIT_MOD_WORKER_NETIO_H

#include <boost/asio/ip/tcp.hpp>
#include <mod_worker/mod_work.h>
#include <mod_worker/mod_worker.h>
#include <asio/deadline_timer.hpp>

using boost::asio::ip::tcp;
using boost::asio::ip::address;
using boost::asio::io_context;

class Work_NetIo_Asio : public Work {
public:
    explicit Work_NetIo_Asio(std::shared_ptr<Work> consignor_work)
    : Work(consignor_work)
    {}
    void do_my_part(int yield_param) override { exit(-1);};
    virtual int do_my_part(io_context& io_ctx) = 0;
    void do_work() override {};
};

//struct Work_NetIo_Asio_Wrap : WorkWrap {
//    Work_NetIo_Asio_Wrap(std::shared_ptr<Work_NetIo_Asio> work, std::shared_ptr<Work> sub_work)
//            : WorkWrap(work, sub_work), m_work_asio(work)
//    {}
//    Work_NetIo_Asio_Wrap(std::shared_ptr<Work_NetIo_Asio> work)
//            : WorkWrap(work), m_work_asio(work)
//    {}
//    std::shared_ptr<Work_NetIo_Asio> m_work_asio;
//};

class Work_NetTcpConnect : public Work_NetIo_Asio {
public:
    explicit Work_NetTcpConnect(std::shared_ptr<Work> consignor_work,
                                std::shared_ptr<tcp::socket> socket_to_server,
                                const std::string& addr_str, uint16_t port)
            : Work_NetIo_Asio(consignor_work), m_socket_to_server(socket_to_server)
    {
        boost::asio::ip::address addr = boost::asio::ip::make_address(addr_str);
        m_endpoint = tcp::endpoint(addr, port);
    }
    int do_my_part(io_context& io_ctx) override;
    tcp::endpoint m_endpoint;
    std::shared_ptr<tcp::socket> m_socket_to_server = nullptr;
};

class Work_NetTcpAccept : public Work_NetIo_Asio {
public:
    explicit Work_NetTcpAccept(std::shared_ptr<Work> consignor_work)
            : Work_NetIo_Asio(consignor_work)
    {}
    int do_my_part(io_context& io_ctx) override;
    tcp::endpoint m_bind_endpoint;
    std::shared_ptr<tcp::acceptor> m_acceptor;
    std::shared_ptr<tcp::socket> m_socket_to_a_client;
};

class Work_NetTcpIn : public Work_NetIo_Asio {
public:
    explicit Work_NetTcpIn(std::shared_ptr<Work> consignor_work,
                           std::shared_ptr<tcp::socket> socket, char* buf, size_t buf_sz)
            : Work_NetIo_Asio(consignor_work), m_socket(socket), in_buf(buf, buf_sz)
    {}
    int do_my_part(io_context& io_ctx) override;
    boost::asio::mutable_buffer in_buf;
    std::shared_ptr<tcp::socket> m_socket;
};

class Work_NetTcpOut : public Work_NetIo_Asio {
public:
    explicit Work_NetTcpOut(std::shared_ptr<Work> consignor_work,
                            std::shared_ptr<tcp::socket> socket, char* buf, size_t buf_sz)
            : Work_NetIo_Asio(consignor_work), m_socket(socket), out_buf(buf, buf_sz)
    {}
    int do_my_part(io_context& io_ctx) override;
    boost::asio::mutable_buffer out_buf;
    std::shared_ptr<tcp::socket> m_socket;
};

class Work_TimerWaitUntil : public Work_NetIo_Asio {
public:
    explicit Work_TimerWaitUntil(std::shared_ptr<Work> consignor_work,
                                 std::shared_ptr<boost::asio::deadline_timer> timer)
    : Work_NetIo_Asio(consignor_work), m_timer(timer) {}
    int do_my_part(io_context& io_ctx) override;
    std::shared_ptr<boost::asio::deadline_timer> m_timer;

};

class Work_TimerWaitFor : public Work_NetIo_Asio {
public:
    explicit Work_TimerWaitFor(std::shared_ptr<Work> consignor_work,
                               std::shared_ptr<boost::asio::deadline_timer> timer)
    : Work_NetIo_Asio(consignor_work), m_timer(timer) {}
    int do_my_part(io_context& io_ctx) override;
    std::shared_ptr<boost::asio::deadline_timer> m_timer;
};

class Work_TimerCancel : public Work_NetIo_Asio {
public:
    explicit Work_TimerCancel(std::shared_ptr<Work> consignor_work,
                              std::shared_ptr<boost::asio::deadline_timer> timer)
    : Work_NetIo_Asio(consignor_work), m_timer(timer) {}
    int do_my_part(io_context& io_ctx) override;
    std::shared_ptr<boost::asio::deadline_timer> m_timer;
};

class Worker_NetIo : public Worker {
public:
    void run() override;
    int add_work(std::shared_ptr<WorkWrap> work) override { exit(-1); };
    int add_work(std::shared_ptr<Work_NetIo_Asio> work);
//    void pop_queue(boost::posix_time::microseconds interval);
    void wait_worker_started() override;
    io_context m_io_ctx;
};

#endif //CPP_TOOLKIT_MOD_WORKER_NETIO_H
