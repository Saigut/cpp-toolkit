#ifndef CPP_TOOLKIT_MOD_CO_NET_H
#define CPP_TOOLKIT_MOD_CO_NET_H

#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/io_context.hpp>

#include <mod_common/os_compat.h>
#include <mod_common/log.h>
#include <mod_common/expect.h>
#include "mod_coroutine.h"

namespace {
    using boost::asio::ip::tcp;
    using boost::asio::io_context;

    class cppt_co_tcp_socket {
    public:
        cppt_co_tcp_socket(tcp::socket&& socket)
        : m_socket(std::move(socket)) {}
        ssize_t write_some(uint8_t* str_buf, size_t str_len) {
            ssize_t wrote_size;
            boost::asio::const_buffer out_buf{ str_buf, str_len };
            auto wrap_func = [&](std::function<void()>&& co_cb) {
                m_socket.async_write_some(out_buf, [&, co_cb](
                        const boost::system::error_code& ec,
                        std::size_t wrote_b_num)
                {
                    check_ec(ec, "write_some");
                    wrote_size = ec ? -1 : (ssize_t)wrote_b_num;
                    co_cb();
                });
            };
            cppt_co_yield(wrap_func);
            return wrote_size;
        }
        ssize_t read_some(uint8_t* recv_buf, size_t buf_sz) {
            ssize_t read_size;
            boost::asio::mutable_buffer in_buf{ recv_buf, buf_sz };
            auto wrap_func = [&](std::function<void()>&& co_cb) {
                m_socket.async_read_some(in_buf, [&, co_cb](
                        const boost::system::error_code& ec,
                        std::size_t read_b_num)
                {
                    check_ec(ec, "read_some");
                    read_size = ec ? -1 : (ssize_t)read_b_num;
                    co_cb();
                });
            };
            cppt_co_yield(wrap_func);
            return read_size;
        }
        virtual bool write(uint8_t* buf, size_t data_sz) {
            size_t remain_data_sz = data_sz;
            ssize_t ret;
            while (remain_data_sz > 0) {
                ret = write_some(buf + (data_sz - remain_data_sz), remain_data_sz);
                if (ret < 0) {
                    log_error("write_some failed! ret: %zd", ret);
                    return false;
                } else if (ret > remain_data_sz) {
                    log_error("write_some failed! ret: %zd, remain_data_sz: %zu", ret, remain_data_sz);
                    return false;
                } else {
                    remain_data_sz -= ret;
                }
            }
            return true;
        }
        virtual bool read(uint8_t* buf, size_t data_sz) {
            size_t remain_data_sz = data_sz;
            ssize_t ret;
            while (remain_data_sz > 0) {
                ret = read_some(buf + (data_sz - remain_data_sz), remain_data_sz);
                if (ret < 0) {
                    log_error("read_some failed! ret: %zd", ret);
                    return false;
                } else if (ret > remain_data_sz) {
                    log_error("read_some failed! ret: %zd, remain_data_sz: %zu", ret, remain_data_sz);
                    return false;
                } else {
                    remain_data_sz -= ret;
                }
            }
            return true;
        }
        void close() {
            m_socket.close();
        }
    private:
        tcp::socket m_socket;
    };

    class cppt_co_tcp_socket_builder {
    public:
        explicit cppt_co_tcp_socket_builder(io_context& io_ctx)
        : m_io_ctx(io_ctx), m_acceptor(io_ctx)
        {}
        std::shared_ptr<cppt_co_tcp_socket> connect(const std::string& addr_str, uint16_t port) {
            int ret = -1;
            boost::asio::ip::address addr = boost::asio::ip::make_address(addr_str);
            tcp::endpoint endpoint = tcp::endpoint(addr, port);
            auto socket_to_server = tcp::socket(m_io_ctx);
            auto wrap_func = [&](std::function<void()>&& co_cb) {
                socket_to_server.async_connect(
                        endpoint,
                        [&, co_cb](const boost::system::error_code& ec) {
                            check_ec(ec, "connect");
                            ret = ec ? -1 : 0;
                            co_cb();
                        });
            };
            cppt_co_yield(wrap_func);
            if (ret != 0) {
                return nullptr;
            }
            return std::make_shared<cppt_co_tcp_socket>(std::move(socket_to_server));
        }
        bool listen(const std::string& local_addr_str, uint16_t local_port) {
            if (m_acceptor.is_open()) {
                return true;
            }
            if (0 != listen_internal(local_addr_str, local_port)) {
                m_acceptor.close();
                return false;
            }
            return true;
        }
        std::shared_ptr<cppt_co_tcp_socket> accept() {
            int ret;
            tcp::socket client_socket(m_io_ctx);
            auto wrap_func = [&](std::function<void()>&& co_cb) {
                m_acceptor.async_accept([&, co_cb](const boost::system::error_code& ec,
                                            tcp::socket peer) {
                    check_ec(ec, "accept");
                    if (!ec) {
                        client_socket = std::move(peer);
                    }
                    ret = ec ? -1 : 0;
                    co_cb();
                });
            };
            cppt_co_yield(wrap_func);
            if (0 != ret) {
                log_error("failed to accept!");
                return nullptr;
            }
            boost::system::error_code ec;
            tcp::endpoint remote_ep = client_socket.remote_endpoint(ec);
            check_ec_ret_val(ec, nullptr, "failed to get remote_endpoint");
//            log_info("New client, ip: %s, port: %u", remote_ep.address().to_string().c_str(),
//                     remote_ep.port());
            return std::make_shared<cppt_co_tcp_socket>(std::move(client_socket));
        }
    private:
        int listen_internal(const std::string& local_addr_str, uint16_t local_port) {
            boost::system::error_code ec;
            // server endpoint
            boost::asio::ip::address addr = boost::asio::ip::make_address(local_addr_str, ec);
            check_ec_ret_val(ec, -1, "make_address");
            tcp::endpoint endpoint(addr, local_port);
            // acceptor
            boost::asio::ip::tcp::acceptor::reuse_address reuse_address_option(true);
            m_acceptor.open(endpoint.protocol(), ec);
            check_ec_ret_val(ec, -1, "acceptor open");
            m_acceptor.set_option(reuse_address_option, ec);
            check_ec_ret_val(ec, -1, "acceptor set_option");
            m_acceptor.bind(endpoint, ec);
            check_ec_ret_val(ec, -1, "acceptor bind");
            m_acceptor.listen(boost::asio::socket_base::max_listen_connections, ec);
            check_ec_ret_val(ec, -1, "acceptor listen");
            return 0;
        }
        io_context& m_io_ctx;
        tcp::acceptor m_acceptor;
    };
}


#endif //CPP_TOOLKIT_MOD_CO_NET_H
