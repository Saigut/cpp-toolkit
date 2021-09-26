#ifndef CPP_TOOLKIT_MOD_WORKUTILS_H
#define CPP_TOOLKIT_MOD_WORKUTILS_H

#include <mod_worker/mod_work.h>
#include <mod_worker/mod_worker_netio.h>

#include <mod_common/expect.h>
#include <asio/deadline_timer.hpp>

namespace WorkUtils {
    class WorkUtils {
    public:
        explicit WorkUtils(Worker* worker, std::shared_ptr<Work> consignor_work, WorkingPoint& wp)
        : m_worker(worker), m_consignor_work(consignor_work), m_wp(wp) {}
        void set_consignor_work(std::shared_ptr<Work> consignor_work) {
            m_consignor_work = consignor_work;
            m_wp = std::move(m_consignor_work->m_wp);
        }
    protected:
        WorkingPoint& m_wp;
        std::shared_ptr<Work> m_consignor_work = nullptr;
        Worker* m_worker = nullptr;
    };

    class TcpSocket : public WorkUtils {
    public:
        explicit TcpSocket(Worker* worker, std::shared_ptr<Work> consignor_work, WorkingPoint& wp)
        : WorkUtils(worker, consignor_work, wp) {}
        virtual int write(char* str_buf, size_t str_len) = 0;
        virtual int read(char* recv_buf, size_t buf_sz, size_t& recv_data_sz) = 0;
    };

    class TcpSocketConnector : public TcpSocket {
    public:
        explicit TcpSocketConnector(Worker* worker, std::shared_ptr<Work> consignor_work, WorkingPoint& wp)
        : TcpSocket(worker, consignor_work, wp) {}
        virtual int connect(const std::string& addr_str, uint16_t port) = 0;
    };
    class TcpAcceptor : public WorkUtils {
    public:
        explicit TcpAcceptor(Worker* worker, std::shared_ptr<Work> consignor_work, WorkingPoint& wp)
                : WorkUtils(worker, consignor_work, wp) {}
        virtual int listen(const std::string& addr_str, uint16_t port) = 0;
        virtual std::shared_ptr<TcpSocket> accept() = 0;
    };
    class TcpSocket_Asio : public TcpSocket {
    public:
        explicit TcpSocket_Asio(Worker_NetIo* net_io_worker, std::shared_ptr<Work> consignor_work, WorkingPoint& wp,
                                std::shared_ptr<tcp::socket> socket,
                                std::string& peer_addr,
                                uint16_t peer_port)
        : TcpSocket(net_io_worker, consignor_work, wp),
        m_net_io_worker(net_io_worker),
        m_socket(socket),
//        m_peer_addr(peer_addr),
        m_peer_port(peer_port)
        {}
        int write(char* str_buf, size_t str_len) override {
            auto work_out = std::make_shared<Work_NetTcpOut>(m_consignor_work, m_socket, str_buf, str_len);
            expect_ret_val(0 == m_net_io_worker->add_work(std::static_pointer_cast<Work_NetIo_Asio>(work_out)), -1);
            expect_ret_val(m_wp.wp_yield(0), -1);
            return m_wp.m_yield_param;
        }
        int read(char* recv_buf, size_t buf_sz, size_t& recv_data_sz) override {
            auto work_in = std::make_shared<Work_NetTcpIn>(m_consignor_work, m_socket, recv_buf, buf_sz);
            expect_ret_val(0 == m_net_io_worker->add_work(std::static_pointer_cast<Work_NetIo_Asio>(work_in)), -1);
            expect_ret_val(m_wp.wp_yield(0), -1);
            if (m_wp.m_yield_param < 0) {
                return -1;
            }
            recv_data_sz = m_wp.m_yield_param;
            return m_wp.m_yield_param;
        }
    private:
        Worker_NetIo* m_net_io_worker = nullptr;
        std::shared_ptr<tcp::socket> m_socket = nullptr;
        std::string m_peer_addr;
        uint16_t m_peer_port;
    };
    class TcpSocketConnector_Asio : public TcpSocketConnector {
    public:
        explicit TcpSocketConnector_Asio(Worker_NetIo* net_io_worker, std::shared_ptr<Work> consignor_work, WorkingPoint& wp)
        : TcpSocketConnector(net_io_worker, consignor_work, wp), m_net_io_worker(net_io_worker) {
            m_socket_to_server = std::make_shared<tcp::socket>(net_io_worker->m_io_ctx);
        }
        ~TcpSocketConnector_Asio() { m_socket_to_server->close(); }
        int connect(const std::string& addr_str, uint16_t port) override;
        int write(char* str_buf, size_t str_len) override;
        int read(char* recv_buf, size_t buf_sz, size_t& recv_data_sz) override;
    private:
        Worker_NetIo* m_net_io_worker = nullptr;
        std::shared_ptr<tcp::socket> m_socket_to_server = nullptr;
    };
    class TcpAcceptor_Asio : public TcpAcceptor {
    public:
        explicit TcpAcceptor_Asio(Worker_NetIo* net_io_worker, std::shared_ptr<Work> consignor_work, WorkingPoint& wp)
        : TcpAcceptor(net_io_worker, consignor_work, wp), m_net_io_worker(net_io_worker) {
            m_acceptor = std::make_shared<tcp::acceptor>(m_net_io_worker->m_io_ctx);
        }
        int listen(const std::string& addr_str, uint16_t port) override {
            boost::system::error_code ec;

            // server endpoint
            boost::asio::ip::address addr = boost::asio::ip::make_address(addr_str, ec);
            check_ec_ret_val(ec, -1, "make_address");
            tcp::endpoint endpoint(addr, port);

            // acceptor
            boost::asio::ip::tcp::acceptor::reuse_address reuse_address_option(true);
            m_acceptor->open(endpoint.protocol(), ec);
            check_ec_ret_val(ec, -1, "acceptor open");
            m_acceptor->set_option(reuse_address_option, ec);
            check_ec_ret_val(ec, -1, "acceptor set_option");
            m_acceptor->bind(endpoint, ec);
            check_ec_ret_val(ec, -1, "acceptor bind");

            m_acceptor->listen(boost::asio::socket_base::max_listen_connections, ec);
            check_ec_ret_val(ec, -1, "acceptor listen");

            return 0;
        }
        std::shared_ptr<TcpSocket> accept() override {

            auto work_acceptor = std::make_shared<Work_NetTcpAccept>(m_consignor_work, m_acceptor);
            expect_ret_val(0 == m_net_io_worker->add_work(std::static_pointer_cast<Work_NetIo_Asio>(work_acceptor)),
                           nullptr);

            expect_ret_val(m_wp.wp_yield(0), nullptr);
            if (0 != m_wp.m_yield_param) {
                log_error("failed to accept!");
                return nullptr;
            }

            boost::system::error_code ec;
            std::shared_ptr<tcp::socket> client_socket = work_acceptor->m_socket_to_a_client;
            work_acceptor->m_socket_to_a_client.reset();

            tcp::endpoint remote_ep = client_socket->remote_endpoint(ec);
            check_ec_ret_val(ec, nullptr, "failed to get remote_endpoint");

            log_info("New client, ip: %s, port: %u",
                     remote_ep.address().to_string().c_str(),
                     remote_ep.port());
            std::string peer_addr(remote_ep.address().to_string()) ;
            return std::make_shared<TcpSocket_Asio>(m_net_io_worker,
                                                    m_consignor_work, m_consignor_work->m_wp,
                                                    client_socket,
                                                    peer_addr, remote_ep.port());
        }
    private:
        Worker_NetIo* m_net_io_worker = nullptr;
        std::shared_ptr<tcp::acceptor> m_acceptor = nullptr;
    };

    class Timer : public WorkUtils {
    public:
        explicit Timer(Worker* worker, std::shared_ptr<Work> consignor_work, WorkingPoint& wp)
        : WorkUtils(worker, consignor_work, wp) {}
        virtual int wait_until(unsigned ts_ms) = 0;
        virtual int wait_for(unsigned ts_ms) = 0;
        virtual int cancel() = 0;
    };
    class Timer_Asio : public Timer {
    public:
        explicit Timer_Asio(Worker_NetIo* worker, std::shared_ptr<Work> consignor_work, WorkingPoint& wp)
        : Timer(worker, consignor_work, wp), m_net_io_worker(worker) {
            m_timer = std::make_shared<boost::asio::deadline_timer>(worker->m_io_ctx);
        }
        int wait_until(unsigned ts_ms) override ;
        int wait_for(unsigned ts_ms) override ;
        int cancel() override ;
    private:
        Worker_NetIo* m_net_io_worker = nullptr;
        std::shared_ptr<boost::asio::deadline_timer> m_timer;
    };
}

#endif //CPP_TOOLKIT_MOD_WORKUTILS_H
