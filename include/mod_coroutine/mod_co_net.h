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
    private:
        tcp::socket m_socket;
    };

    class cppt_co_tcp_socket_builder {
    public:
        explicit cppt_co_tcp_socket_builder(io_context& io_ctx)
        : m_io_ctx(io_ctx), m_acceptor(io_ctx)
        {}
        cppt_co_tcp_socket&& connect(const std::string& addr_str, uint16_t port);
        bool listen(const std::string& local_addr_str, uint16_t local_port);
        cppt_co_tcp_socket&& accept();
    private:
        io_context& m_io_ctx;
        tcp::acceptor m_acceptor;
    };
}


#endif //CPP_TOOLKIT_MOD_CO_NET_H
