#include <mod_worker/mod_workutils.h>

#include <mod_common/expect.h>

#define CHECK_YIELD_PARAM(_yield_param) \
do { \
    if (0 != (_yield_param)) { \
        if (1 == (_yield_param)) { \
            log_error("timed out!"); \
            return -1; \
        } else { \
            log_error("sub work failed!"); \
            return -1; \
        } \
    } \
} while (0)

#define CHECK_YIELD_PARAM_TIMER(_yield_param, _timer) \
do { \
    if (0 != (_yield_param)) { \
        if (1 == (_yield_param)) { \
            log_error("timed out!"); \
            return -1; \
        } else { \
            log_error("sub work failed!"); \
            _timer->cancel(); \
            return -1; \
        } \
    } \
    _timer->cancel();\
} while (0)

namespace WorkUtils {
    int TcpSocketConnector_Asio::connect(const std::string& addr_str, uint16_t port) {
        int ret;
        auto work_back = m_work_back;
        boost::asio::ip::address addr = boost::asio::ip::make_address(addr_str);
        tcp::endpoint endpoint = tcp::endpoint(addr, port);
        m_socket_to_server->async_connect(endpoint,
                                          [&ret, work_back](const boost::system::error_code& ec) {
                                              check_ec(ec, "connect");
                                              ret = ec ? -1 : 0;
                                              if (work_back) {
                                                  work_back();
                                              }
                                          });
        m_work_yield();
        return ret;
    }
    int TcpSocketConnector_Asio::read(char *recv_buf, size_t buf_sz, size_t &recv_data_sz) {
        int ret;
        auto work_back = m_work_back;
        boost::asio::mutable_buffer in_buf(recv_buf, buf_sz);
        m_socket_to_server->async_read_some(in_buf,
                                            [&ret, work_back](const boost::system::error_code& ec,
                                                    std::size_t read_b_num) {
                                                check_ec(ec, "read_some");
                                                ret = ec ? -1 : (int)read_b_num;
                                                if (work_back) {
                                                    work_back();
                                                }
                                            });
        m_work_yield();
        if (ret < 0) {
            return -1;
        }
        recv_data_sz = ret;
        return 0;
    }
    int TcpSocketConnector_Asio::write(char *str_buf, size_t str_len) {
        int ret;
        auto work_back = m_work_back;
        boost::asio::mutable_buffer out_buf(str_buf, str_len);
        m_socket_to_server->async_write_some(out_buf,
                                             [&ret, work_back](const boost::system::error_code& ec,
                                                     std::size_t write_b_num) {
                                                 check_ec(ec, "write_some");
                                                 ret = ec ? -1 : (int)write_b_num;
                                                 if (work_back) {
                                                     work_back();
                                                 }
                                             });
        m_work_yield();
        if (str_len != ret) {
            return -1;
        }
        return 0;
    }

    int Timer_Asio::wait_until(unsigned int ts_ms) {
        return -1;
    }
    int Timer_Asio::wait_for(unsigned int ts_ms) {
        m_timer->expires_from_now(boost::posix_time::milliseconds(ts_ms));
        auto work_timer = std::make_shared<Work_TimerWaitFor>(m_consignor_work, m_timer);
        expect_ret_val(0 == m_net_io_worker->add_work(std::static_pointer_cast<Work_NetIo_Asio>(work_timer)), -1);
        expect_ret_val(m_wp.wp_yield(0), -1);
        return 0;
    }
    int Timer_Asio::cancel() {
        m_timer->cancel();
        return 0;
    }
}
