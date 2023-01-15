#ifndef CPP_TOOLKIT_MOD_COR_NET_HPP
#define CPP_TOOLKIT_MOD_COR_NET_HPP

#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/io_context.hpp>

#include <mod_common/os_compat.hpp>
#include <mod_common/log.hpp>
#include <mod_common/expect.hpp>
#include "mod_cor.hpp"

namespace cppt {
    using boost::asio::ip::tcp;
    using boost::asio::io_context;

    class cor_tcp_socket_t {
    public:
        cor_tcp_socket_t(tcp::socket&& socket)
        : m_socket(std::move(socket)) {}
        ssize_t write_some(uint8_t* str_buf, size_t str_len) {
            ssize_t wrote_size;
            boost::asio::const_buffer out_buf{ str_buf, str_len };
            auto wrap_func = [&](std::function<void()>&& resume_f) {
                m_socket.async_write_some(out_buf, [&, resume_f](
                        const boost::system::error_code& ec,
                        std::size_t wrote_b_num)
                {
//                    check_ec(ec, "write_some");
                    wrote_size = ec ? -1 : (ssize_t)wrote_b_num;
                    resume_f();
                });
            };
            cppt::cor_yield(wrap_func);
//            auto timeout_func = [&](){
//                m_socket.close();
//            };
//            int ret = cppt::cor_yield(wrap_func, 3000, timeout_func);
//            if (0 != ret) {
//                log_error("async error occurred. ret: %d", ret);
//                return -1;
//            }
            return wrote_size;
        }
        ssize_t read_some(uint8_t* recv_buf, size_t buf_sz) {
            ssize_t read_size;
            boost::asio::mutable_buffer in_buf{ recv_buf, buf_sz };
            auto wrap_func = [&](std::function<void()>&& resume_f) {
                m_socket.async_read_some(in_buf, [&, resume_f](
                        const boost::system::error_code& ec,
                        std::size_t read_b_num)
                {
//                    check_ec(ec, "read_some");
                    read_size = ec ? -1 : (ssize_t)read_b_num;
                    resume_f();
                });
            };
            cppt::cor_yield(wrap_func);
//            auto timeout_func = [&](){
//                m_socket.close();
//            };
//            int ret = cppt::cor_yield(wrap_func, 3000, timeout_func);
//            if (0 != ret) {
//                log_error("async error occurred. ret: %d", ret);
//                return -1;
//            }
            return read_size;
        }
        bool write(uint8_t* buf, size_t data_sz) {
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
        bool read(uint8_t* buf, size_t data_sz) {
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

    class cor_tcp_socket_builder {
    public:
        explicit cor_tcp_socket_builder(io_context& io_ctx)
        : m_io_ctx(io_ctx), m_acceptor(io_ctx)
        {}
        std::shared_ptr<cor_tcp_socket_t> connect(const std::string& addr_str, uint16_t port) {
            int ret = -1;
            boost::asio::ip::address addr = boost::asio::ip::make_address(addr_str);
            tcp::endpoint endpoint = tcp::endpoint(addr, port);
            auto socket_to_server = tcp::socket(m_io_ctx);
            auto wrap_func = [&](std::function<void()>&& resume_f) {
                socket_to_server.async_connect(
                        endpoint,
                        [&, resume_f](const boost::system::error_code& ec) {
                            check_ec(ec, "connect");
                            ret = ec ? -1 : 0;
                            resume_f();
                        });
            };
            cppt::cor_yield(wrap_func);
//            auto timeout_func = [&](){
//            };
//            ret = cppt::cor_yield(wrap_func, 3000, timeout_func);
//            if (0 != ret) {
//                log_error("async error occurred. ret: %d", ret);
//                return nullptr;
//            }
            if (ret != 0) {
                return nullptr;
            }
            return std::make_shared<cor_tcp_socket_t>(std::move(socket_to_server));
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
        cor_tcp_socket_t* accept(io_context& io_ctx) {
            int ret;
            tcp::socket client_socket(io_ctx);
            auto wrap_func = [&](std::function<void()>&& resume_f) {
                m_acceptor.async_accept(client_socket, [&, resume_f](const boost::system::error_code& ec) {
                    check_ec(ec, "accept");
                    ret = ec ? -1 : 0;
                    resume_f();
                });
            };
            cppt::cor_yield(wrap_func);
            if (0 != ret) {
                log_error("failed to accept!");
                return nullptr;
            }
            return new cor_tcp_socket_t{std::move(client_socket)};
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


#endif //CPP_TOOLKIT_MOD_COR_NET_HPP
