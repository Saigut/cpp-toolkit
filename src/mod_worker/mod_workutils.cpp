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

namespace WorkUtils {
    int TcpSocketConnector_Asio::connect(const std::string& addr_str, uint16_t port) {
        auto work_connect = std::make_shared<Work_NetTcpConnect>(m_consignor_work, m_socket_to_server, addr_str, port);
        expect_ret_val(0 == m_net_io_worker->add_work(std::static_pointer_cast<Work_NetIo_Asio>(work_connect)), -1);
        expect_ret_val(m_wp.yield(0), -1);
        CHECK_YIELD_PARAM(m_wp.m_yield_param);
        return 0;
    }
    int TcpSocketConnector_Asio::read(char *recv_buf, size_t buf_sz, size_t &recv_data_sz) {
        auto work_in = std::make_shared<Work_NetTcpIn>(m_consignor_work, m_socket_to_server, recv_buf, buf_sz);
        expect_ret_val(0 == m_net_io_worker->add_work(std::static_pointer_cast<Work_NetIo_Asio>(work_in)), -1);
        expect_ret_val(m_wp.yield(0), -1);
        CHECK_YIELD_PARAM(m_wp.m_yield_param);
        recv_data_sz = work_in->in_buf.size();
        return 0;
    }
    int TcpSocketConnector_Asio::write(char *str_buf, size_t str_len) {
        auto work_out = std::make_shared<Work_NetTcpOut>(m_consignor_work, m_socket_to_server, str_buf, str_len);
        expect_ret_val(0 == m_net_io_worker->add_work(std::static_pointer_cast<Work_NetIo_Asio>(work_out)), -1);
        expect_ret_val(m_wp.yield(0), -1);
        CHECK_YIELD_PARAM(m_wp.m_yield_param);
        return 0;
    }

    int Timer_Asio::wait_until(unsigned int ts_ms) {
        return -1;
    }
    int Timer_Asio::wait_for(unsigned int ts_ms) {
        return -1;
    }
    int Timer_Asio::cancel() {
        return -1;
    }
}
