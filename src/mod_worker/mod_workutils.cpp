#include <mod_worker/mod_workutils.h>

#include <mod_common/expect.h>

namespace WorkUtils {
    int TcpSocketConnector_Asio::connect(const std::string& addr_str, uint16_t port) {
        auto work_connect = new Work_NetTcpConnect(m_consignor_work, m_socket_to_server, addr_str, port);
        auto work_connect_wrap = new Work_NetIo_Asio_Wrap(work_connect);
        expect_ret_val(0 == m_net_io_worker->add_work(work_connect_wrap), -1);
        expect_ret_val(m_wp.yield(), -1);
        expect_ret_val(0 == work_connect->finish_ret, -1);
        return 0;
    }
    int TcpSocketConnector_Asio::read(char *recv_buf, size_t buf_sz, size_t &recv_data_sz) {
        auto work_in = new Work_NetTcpIn(m_consignor_work, m_socket_to_server, recv_buf, buf_sz);
        auto work_in_wrap = new Work_NetIo_Asio_Wrap(work_in);
        expect_ret_val(0 == m_net_io_worker->add_work(work_in_wrap), -1);
        expect_ret_val(m_wp.yield(), -1);
        expect_ret_val(0 == work_in->finish_ret, -1);
        recv_data_sz = work_in->in_buf.size();
        return 0;
    }
    int TcpSocketConnector_Asio::write(char *str_buf, size_t str_len) {
        auto work_out = new Work_NetTcpOut(m_consignor_work, m_socket_to_server, str_buf, str_len);
        auto work_out_wrap = new Work_NetIo_Asio_Wrap(work_out);
        expect_ret_val(0 == m_net_io_worker->add_work(work_out_wrap), -1);
        expect_ret_val(m_wp.yield(), -1);
        expect_ret_val(0 == work_out->finish_ret, -1);
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
