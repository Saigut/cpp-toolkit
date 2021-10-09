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
    int TcpSocketConnector_Asio::connect(std::shared_ptr<Work> consignor_work, const std::string& addr_str, uint16_t port) {
        auto work_connect = std::make_shared<Work_NetTcpConnect>(consignor_work, m_socket_to_server, addr_str, port);
        expect_ret_val(0 == m_net_io_worker->add_work(std::static_pointer_cast<Work_NetIo_Asio>(work_connect)), -1);

//        auto timer = std::make_shared<boost::asio::deadline_timer>(m_net_io_worker->m_io_ctx);
//        timer->expires_from_now(boost::posix_time::milliseconds(5*1000));
//        auto work_timer = std::make_shared<Work_TimerWaitFor>(m_consignor_work, timer);
//        expect_ret_val(0 == m_net_io_worker->add_work(std::static_pointer_cast<Work_NetIo_Asio>(work_timer)), -1);

        expect_ret_val(consignor_work->m_wp.wp_yield(0), -1);
        CHECK_YIELD_PARAM(consignor_work->m_wp.m_yield_param);
//        CHECK_YIELD_PARAM_TIMER(m_wp.m_yield_param, timer);
        return 0;
    }
    int TcpSocketConnector_Asio::read(std::shared_ptr<Work> consignor_work, char *recv_buf, size_t buf_sz, size_t &recv_data_sz) {
        auto work_in = std::make_shared<Work_NetTcpIn>(consignor_work, m_socket_to_server, recv_buf, buf_sz);
        expect_ret_val(0 == m_net_io_worker->add_work(std::static_pointer_cast<Work_NetIo_Asio>(work_in)), -1);

//        auto timer = std::make_shared<boost::asio::deadline_timer>(m_net_io_worker->m_io_ctx);
//        timer->expires_from_now(boost::posix_time::milliseconds(5*1000));
//        auto work_timer = std::make_shared<Work_TimerWaitFor>(m_consignor_work, timer);
//        expect_ret_val(0 == m_net_io_worker->add_work(std::static_pointer_cast<Work_NetIo_Asio>(work_timer)), -1);

        expect_ret_val(consignor_work->m_wp.wp_yield(0), -1);
//        CHECK_YIELD_PARAM_TIMER(m_wp.m_yield_param, timer);

        if (consignor_work->m_wp.m_yield_param < 0) {
            return -1;
        }
        recv_data_sz = consignor_work->m_wp.m_yield_param;
        return consignor_work->m_wp.m_yield_param;
    }
    int TcpSocketConnector_Asio::write(std::shared_ptr<Work> consignor_work, char *str_buf, size_t str_len) {
        auto work_out = std::make_shared<Work_NetTcpOut>(consignor_work, m_socket_to_server, str_buf, str_len);
        expect_ret_val(0 == m_net_io_worker->add_work(std::static_pointer_cast<Work_NetIo_Asio>(work_out)), -1);
        expect_ret_val(consignor_work->m_wp.wp_yield(0), -1);
        return consignor_work->m_wp.m_yield_param;
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
