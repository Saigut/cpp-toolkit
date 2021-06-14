#include <mod_worker/mod_workutils.h>

#include <mod_common/expect.h>

namespace WorkUtils {
    int TcpSocketConnector_Asio::connect(const std::string& addr_str, uint16_t port) {
        Work_NetTcpConnect work_connect(m_consignor_work, m_socket_to_server, addr_str, port);
        expect_ret_val(0 == m_net_io_worker->add_work(&work_connect), -1);
        expect_ret_val(m_wp.yield(), -1);
        return 0;
    }
    int TcpSocketConnector_Asio::read(char *recv_buf, size_t buf_sz, size_t &recv_data_sz) {
        Work_NetTcpIn work_in(m_consignor_work, m_socket_to_server, recv_buf, buf_sz);
        expect_ret_val(0 == m_net_io_worker->add_work(&work_in), -1);
        expect_ret_val(m_wp.yield(), -1);
        recv_data_sz = work_in.in_buf.size();
        return 0;
    }
    int TcpSocketConnector_Asio::write(char *str_buf, size_t str_len) {
        Work_NetTcpOut work_out(m_consignor_work, m_socket_to_server, str_buf, str_len);
        expect_ret_val(0 == m_net_io_worker->add_work(&work_out), -1);
        expect_ret_val(m_wp.yield(), -1);
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
