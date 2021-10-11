#ifndef CPP_TOOLKIT_MOD_WORKUTILS_H
#define CPP_TOOLKIT_MOD_WORKUTILS_H

#include <mod_worker/mod_work.h>
#include <mod_worker/mod_worker_netio.h>

#include <mod_common/expect.h>
#include <asio/deadline_timer.hpp>

namespace WorkUtils {
    class WorkUtils {
    public:
        explicit WorkUtils(Worker* worker)
        : m_worker(worker) {}
    protected:
        Worker* m_worker = nullptr;
    };

    class TcpSocketAb : public WorkUtils {
    public:
        explicit TcpSocketAb(Worker* worker)
        : WorkUtils(worker) {}
        virtual int write(std::shared_ptr<Work> consignor_work, char* str_buf, size_t str_len) {
            log_error("Should not call this!");
            return -1;
        }
        virtual int read(std::shared_ptr<Work> consignor_work, char* recv_buf, size_t buf_sz, size_t& recv_data_sz) {
            log_error("Should not call this!");
            return -1;
        }
    };

    class TcpSocketConnector : public TcpSocketAb {
    public:
        explicit TcpSocketConnector(Worker* worker)
        : TcpSocketAb(worker) {}
        virtual int connect(std::shared_ptr<Work> consignor_work, const std::string& addr_str, uint16_t port) = 0;
    };
    class TcpAcceptor : public WorkUtils {
    public:
        explicit TcpAcceptor(Worker* worker)
                : WorkUtils(worker) {}
        virtual int listen(const std::string& addr_str, uint16_t port) = 0;
        virtual std::shared_ptr<TcpSocketAb> accept(std::shared_ptr<Work> consignor_work) = 0;
    };
    class TcpSocket_Asio : public TcpSocketAb {
    public:
        explicit TcpSocket_Asio(Worker_NetIo* net_io_worker,
                                std::shared_ptr<tcp::socket> socket,
                                std::string& peer_addr,
                                uint16_t peer_port)
        : TcpSocketAb((Worker*)net_io_worker),
        m_net_io_worker(net_io_worker),
        m_socket(socket)
        {}
        int write(std::shared_ptr<Work> consignor_work, char* str_buf, size_t str_len) override {
            auto work_out = std::make_shared<Work_NetTcpOut>(consignor_work, m_socket, str_buf, str_len);
            expect_ret_val(0 == m_net_io_worker->add_work(std::static_pointer_cast<Work_NetIo_Asio>(work_out)), -1);
            expect_ret_val(consignor_work->m_wp.wp_yield(0), -1);
            return consignor_work->m_wp.m_yield_param;
        }
        int read(std::shared_ptr<Work> consignor_work, char* recv_buf, size_t buf_sz, size_t& recv_data_sz) override {
            auto work_in = std::make_shared<Work_NetTcpIn>(consignor_work, m_socket, recv_buf, buf_sz);
            expect_ret_val(0 == m_net_io_worker->add_work(std::static_pointer_cast<Work_NetIo_Asio>(work_in)), -1);
            expect_ret_val(consignor_work->m_wp.wp_yield(0), -1);
            if (consignor_work->m_wp.m_yield_param < 0) {
                return -1;
            }
            recv_data_sz = consignor_work->m_wp.m_yield_param;
            return consignor_work->m_wp.m_yield_param;
        }
    private:
        Worker_NetIo* m_net_io_worker = nullptr;
        std::shared_ptr<tcp::socket> m_socket = nullptr;
    };
    class TcpSocketConnector_Asio : public TcpSocketConnector {
    public:
        explicit TcpSocketConnector_Asio(Worker_NetIo* net_io_worker)
        : TcpSocketConnector((Worker*)net_io_worker), m_net_io_worker(net_io_worker) {
            m_socket_to_server = std::make_shared<tcp::socket>(net_io_worker->m_io_ctx);
        }
        ~TcpSocketConnector_Asio() { m_socket_to_server->close(); }
        int connect(std::shared_ptr<Work> consignor_work, const std::string& addr_str, uint16_t port) override;
        int write(std::shared_ptr<Work> consignor_work, char* str_buf, size_t str_len) override;
        int read(std::shared_ptr<Work> consignor_work, char* recv_buf, size_t buf_sz, size_t& recv_data_sz) override;
        Worker_NetIo* m_net_io_worker = nullptr;
        std::shared_ptr<tcp::socket> m_socket_to_server = nullptr;
    };
    class TcpAcceptor_Asio : public TcpAcceptor {
    public:
        explicit TcpAcceptor_Asio(Worker_NetIo* net_io_worker)
        : TcpAcceptor((Worker*)net_io_worker), m_net_io_worker(net_io_worker) {
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
        std::shared_ptr<TcpSocketAb> accept(std::shared_ptr<Work> consignor_work) override {

            auto work_acceptor = std::make_shared<Work_NetTcpAccept>(consignor_work, m_acceptor);
            expect_ret_val(0 == m_net_io_worker->add_work(std::static_pointer_cast<Work_NetIo_Asio>(work_acceptor)),
                           nullptr);

            expect_ret_val(consignor_work->m_wp.wp_yield(0), nullptr);
            if (0 != consignor_work->m_wp.m_yield_param) {
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
                                                    client_socket,
                                                    peer_addr, remote_ep.port());
        }
    private:
        Worker_NetIo* m_net_io_worker = nullptr;
        std::shared_ptr<tcp::acceptor> m_acceptor = nullptr;
    };

    class Timer : public WorkUtils {
    public:
        explicit Timer(Worker* worker, std::shared_ptr<Work> consignor_work)
        : WorkUtils(worker), m_consignor_work(consignor_work), m_wp(consignor_work->m_wp) {}
        virtual int wait_until(unsigned ts_ms) = 0;
        virtual int wait_for(unsigned ts_ms) = 0;
        virtual int cancel() = 0;
    protected:
        std::shared_ptr<Work> m_consignor_work = nullptr;
        WorkingPoint& m_wp;
    };
    class Timer_Asio : public Timer {
    public:
        explicit Timer_Asio(Worker_NetIo* worker, std::shared_ptr<Work> consignor_work)
        : Timer((Worker*)worker, consignor_work), m_net_io_worker(worker) {
            m_timer = std::make_shared<boost::asio::deadline_timer>(worker->m_io_ctx);
        }
        int wait_until(unsigned ts_ms) override ;
        int wait_for(unsigned ts_ms) override ;
        int cancel() override ;
    private:
        Worker_NetIo* m_net_io_worker = nullptr;
        std::shared_ptr<boost::asio::deadline_timer> m_timer;
    };

    class tcp_socket {
    public:
        explicit tcp_socket(std::shared_ptr<TcpSocketAb> tcp_socket)
                : m_tcp_socket(tcp_socket)
        {}
        virtual int write_some(std::shared_ptr<Work> consignor_work, uint8_t* buf, size_t data_sz) {
            return m_tcp_socket->write(consignor_work, (char *)buf, data_sz);
        }
        virtual int read_some(std::shared_ptr<Work> consignor_work, uint8_t* buf, size_t buf_sz) {
            size_t recv_data_sz;
            int ret;
            ret = m_tcp_socket->read(consignor_work, (char *)buf, buf_sz, recv_data_sz);
            return ret;
        }
        virtual bool write(std::shared_ptr<Work> consignor_work, uint8_t* buf, size_t data_sz) {
            size_t remain_data_sz = data_sz;
            int ret;
            while (remain_data_sz > 0) {
                ret = write_some(consignor_work, buf + (data_sz - remain_data_sz), remain_data_sz);
                if (ret <= 0) {
                    log_error("write_some failed! ret: %d", ret);
                    return false;
                } else if (ret > remain_data_sz) {
                    log_error("write_some failed! ret: %d, remain_data_sz: %zu", ret, remain_data_sz);
                    return false;
                } else {
                    remain_data_sz -= ret;
                }
            }
            return true;
        }
        virtual bool read(std::shared_ptr<Work> consignor_work, uint8_t* buf, size_t data_sz) {
            size_t remain_data_sz = data_sz;
            int ret;
            while (remain_data_sz > 0) {
                ret = read_some(consignor_work, buf + (data_sz - remain_data_sz), remain_data_sz);
                if (ret <= 0) {
                    log_error("read_some failed! ret: %d", ret);
                    return false;
                } else if (ret > remain_data_sz) {
                    log_error("read_some failed! ret: %d, remain_data_sz: %zu", ret, remain_data_sz);
                    return false;
                } else {
                    remain_data_sz -= ret;
                }
            }
            return true;
        }
    private:
        std::shared_ptr<TcpSocketAb> m_tcp_socket;
    };

    class channel_ab {
    public:
        channel_ab(std::shared_ptr<tcp_socket> tcp)
                : m_tcp(tcp) {}
        virtual bool send_msg(std::shared_ptr<Work> consignor_work, std::string& msg) = 0;
        virtual bool recv_msg(std::shared_ptr<Work> consignor_work, std::string& msg) = 0;
    protected:
        std::shared_ptr<tcp_socket> m_tcp;
    };

    class channel_builder_ab : public WorkUtils::WorkUtils {
    public:
        explicit channel_builder_ab(Worker_NetIo* worker)
                : WorkUtils((Worker*)worker) {}
        // for client
        virtual std::shared_ptr<channel_ab> connect(std::shared_ptr<Work> consignor_work) = 0;
        // for server
        virtual bool listen() = 0;
        virtual std::shared_ptr<channel_ab> accept(std::shared_ptr<Work> consignor_work) = 0;
    };
}

#endif //CPP_TOOLKIT_MOD_WORKUTILS_H
