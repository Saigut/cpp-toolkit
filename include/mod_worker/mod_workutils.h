#ifndef CPP_TOOLKIT_MOD_WORKUTILS_H
#define CPP_TOOLKIT_MOD_WORKUTILS_H

#include <mod_worker/mod_work.h>
#include <mod_worker/mod_worker_netio.h>

#include <mod_common/expect.h>

namespace WorkUtils {
    class TcpSocketConnector {
    public:
        explicit TcpSocketConnector(WorkingPoint& wp) : m_wp(wp) {}
        virtual int connect(const std::string& addr_str, uint16_t port) = 0;
        virtual int write(char* str_buf, size_t str_len) = 0;
        virtual int read(char* recv_buf, size_t buf_sz, size_t& recv_data_sz) = 0;
    protected:
        WorkingPoint& m_wp;
    };
    class TcpSocketConnector_Asio : public TcpSocketConnector {
    public:
        explicit TcpSocketConnector_Asio(Worker_NetIo* net_io_worker, Work* consignor, WorkingPoint& wp)
        : TcpSocketConnector(wp), m_net_io_worker(net_io_worker), m_consignor(consignor) {
            m_socket_to_server = std::make_shared<tcp::socket>(net_io_worker->m_io_ctx);
        }
        ~TcpSocketConnector_Asio() { m_socket_to_server->close(); }
        int connect(const std::string& addr_str, uint16_t port) override;
        int write(char* str_buf, size_t str_len) override;
        int read(char* recv_buf, size_t buf_sz, size_t& recv_data_sz) override;
    private:
        Work* m_consignor = nullptr;
        Worker_NetIo* m_net_io_worker = nullptr;
        std::shared_ptr<tcp::socket> m_socket_to_server = nullptr;
    };
}

#endif //CPP_TOOLKIT_MOD_WORKUTILS_H
