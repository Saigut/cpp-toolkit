#ifndef CPP_TOOLKIT_MOD_WORKUTILS_H
#define CPP_TOOLKIT_MOD_WORKUTILS_H

#include <mod_worker/mod_work.h>
#include <mod_worker/mod_worker_netio.h>

#include <mod_common/expect.h>
#include <asio/deadline_timer.hpp>

namespace WorkUtils {
    class WorkUtils {
    public:
        explicit WorkUtils(Worker* worker, std::shared_ptr<Work> consignor_work, WorkingPoint& wp)
        : m_worker(worker), m_consignor_work(consignor_work), m_wp(wp) {}
    protected:
        WorkingPoint& m_wp;
        std::shared_ptr<Work> m_consignor_work = nullptr;
        Worker* m_worker = nullptr;
    };

    class TcpSocketConnector : public WorkUtils {
    public:
        explicit TcpSocketConnector(Worker* worker, std::shared_ptr<Work> consignor_work, WorkingPoint& wp)
        : WorkUtils(worker, consignor_work, wp) {}
        virtual int connect(const std::string& addr_str, uint16_t port) = 0;
        virtual int write(char* str_buf, size_t str_len) = 0;
        virtual int read(char* recv_buf, size_t buf_sz, size_t& recv_data_sz) = 0;
    };
    class TcpSocketConnector_Asio : public TcpSocketConnector {
    public:
        explicit TcpSocketConnector_Asio(Worker_NetIo* net_io_worker, std::shared_ptr<Work> consignor_work, WorkingPoint& wp)
        : TcpSocketConnector(net_io_worker, consignor_work, wp), m_net_io_worker(net_io_worker) {
            m_socket_to_server = std::make_shared<tcp::socket>(net_io_worker->m_io_ctx);
        }
        ~TcpSocketConnector_Asio() { m_socket_to_server->close(); }
        int connect(const std::string& addr_str, uint16_t port) override;
        int write(char* str_buf, size_t str_len) override;
        int read(char* recv_buf, size_t buf_sz, size_t& recv_data_sz) override;
    private:
        Worker_NetIo* m_net_io_worker = nullptr;
        std::shared_ptr<tcp::socket> m_socket_to_server = nullptr;
    };

    class Timer : public WorkUtils {
    public:
        explicit Timer(Worker* worker, std::shared_ptr<Work> consignor_work, WorkingPoint& wp)
        : WorkUtils(worker, consignor_work, wp) {}
        virtual int wait_until(unsigned ts_ms) = 0;
        virtual int wait_for(unsigned ts_ms) = 0;
        virtual int cancel() = 0;
    };
    class Timer_Asio : public Timer {
    public:
        explicit Timer_Asio(Worker_NetIo* worker, std::shared_ptr<Work> consignor_work, WorkingPoint& wp)
        : Timer(worker, consignor_work, wp), m_net_io_worker(worker) {
            m_timer = std::make_shared<boost::asio::deadline_timer>(worker->m_io_ctx);
        }
        int wait_until(unsigned ts_ms) override ;
        int wait_for(unsigned ts_ms) override ;
        int cancel() override ;
    private:
        Worker_NetIo* m_net_io_worker = nullptr;
        std::shared_ptr<boost::asio::deadline_timer> m_timer;
    };
}

#endif //CPP_TOOLKIT_MOD_WORKUTILS_H
