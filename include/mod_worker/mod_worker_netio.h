#ifndef CPP_TOOLKIT_MOD_WORKER_NETIO_H
#define CPP_TOOLKIT_MOD_WORKER_NETIO_H

#include <boost/asio/ip/tcp.hpp>
#include <mod_worker/mod_work.h>
#include <mod_worker/mod_worker.h>

using boost::asio::ip::tcp;
using boost::asio::ip::address;
using boost::asio::io_context;

class Work_NetIo_Asio : public Work {
public:
    explicit Work_NetIo_Asio(Work* consignor)
    : Work(consignor)
    {}
    void do_my_part() override { exit(-1);};
    void do_work() override {};
    virtual int do_my_part(io_context& io_ctx) = 0;
};

class Work_NetTcpConnect : public Work_NetIo_Asio {
public:
    explicit Work_NetTcpConnect(Work* consignor)
            : Work_NetIo_Asio(consignor)
    {}
    int do_my_part(io_context& io_ctx) override;
    tcp::endpoint m_endpoint;
    std::shared_ptr<tcp::socket> m_socket_to_server = nullptr;
};

class Work_NetTcpAccept : public Work_NetIo_Asio {
public:
    explicit Work_NetTcpAccept(Work* consignor)
            : Work_NetIo_Asio(consignor)
    {}
    int do_my_part(io_context& io_ctx) override;
    tcp::endpoint m_bind_endpoint;
    std::shared_ptr<tcp::acceptor> m_acceptor;
    std::shared_ptr<tcp::socket> m_socket_to_a_client;
};

class Work_NetTcpIn : public Work_NetIo_Asio {
public:
    explicit Work_NetTcpIn(Work* consignor)
            : Work_NetIo_Asio(consignor)
    {}
    int do_my_part(io_context& io_ctx) override;
    boost::asio::mutable_buffer in_buf;
    std::shared_ptr<tcp::socket> m_socket;
};

class Work_NetTcpOut : public Work_NetIo_Asio {
public:
    explicit Work_NetTcpOut(Work* consignor)
            : Work_NetIo_Asio(consignor)
    {}
    int do_my_part(io_context& io_ctx) override;
    boost::asio::mutable_buffer out_buf;
    std::shared_ptr<tcp::socket> m_socket;
};

class Worker_NetIo : public Worker {
public:
    void run() override;
//    int add_work(Work* work) override { exit(-1); };
//    int add_work(Work_NetIo_Asio* work);
    void pop_queue(boost::posix_time::microseconds interval);
    io_context m_io_ctx;
};

#endif //CPP_TOOLKIT_MOD_WORKER_NETIO_H
