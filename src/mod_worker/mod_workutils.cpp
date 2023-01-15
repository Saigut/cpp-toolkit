#include <mod_worker/mod_workutils.hpp>

#include <mod_common/expect.hpp>

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
    int Timer_Asio::wait_until(unsigned int ts_ms) {
        return -1;
    }
    int Timer_Asio::wait_for(unsigned int ts_ms) {
        int ret;
        auto work_back = m_co_cbs.m_work_back;
        m_timer->expires_from_now(boost::posix_time::milliseconds(ts_ms));
        m_timer->async_wait([&ret, work_back](const boost::system::error_code& ec) {
            check_ec(ec, "timer async_wait");
            ret = ec ? -1 : 0;
            if (work_back) {
                work_back();
            }
        });
        m_co_cbs.m_work_yield();
        return 0;
    }
    int Timer_Asio::cancel() {
        m_timer->cancel();
        return 0;
    }
}
