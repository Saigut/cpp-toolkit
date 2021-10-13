#ifndef CPP_TOOLKIT_MOD_WORKUTILS_H
#define CPP_TOOLKIT_MOD_WORKUTILS_H

#include <mod_worker/mod_work.h>
#include <mod_worker/mod_worker_netio.h>

#include <mod_common/expect.h>
#include <boost/asio/deadline_timer.hpp>
#include <boost/endian.hpp>

namespace WorkUtils {
    class WorkUtils {
    public:
        explicit WorkUtils(Worker* worker)
        : m_worker(worker) {}
    protected:
        Worker* m_worker = nullptr;
    };

    class TcpSocketAb : public WorkUtils {
    public:
        explicit TcpSocketAb(Worker* worker)
        : WorkUtils(worker) {}
        virtual int write(std::shared_ptr<Work> consignor_work, char* str_buf, size_t str_len) {
            log_error("Should not call this!");
            return -1;
        }
        virtual int read(std::shared_ptr<Work> consignor_work, char* recv_buf, size_t buf_sz, size_t& recv_data_sz) {
            log_error("Should not call this!");
            return -1;
        }
    };

    class TcpSocketConnector : public TcpSocketAb {
    public:
        explicit TcpSocketConnector(Worker* worker)
        : TcpSocketAb(worker) {}
        virtual int connect(std::shared_ptr<Work> consignor_work, const std::string& addr_str, uint16_t port) = 0;
    };
    class TcpAcceptor : public WorkUtils {
    public:
        explicit TcpAcceptor(Worker* worker)
                : WorkUtils(worker) {}
        virtual int listen(const std::string& addr_str, uint16_t port) = 0;
        virtual std::shared_ptr<TcpSocketAb> accept(std::shared_ptr<Work> consignor_work) = 0;
    };
    class TcpSocket_Asio : public TcpSocketAb {
    public:
        explicit TcpSocket_Asio(Worker_NetIo* net_io_worker,
                                std::shared_ptr<tcp::socket> socket,
                                std::string& peer_addr,
                                uint16_t peer_port)
        : TcpSocketAb((Worker*)net_io_worker),
        m_net_io_worker(net_io_worker),
        m_socket(socket)
        {}
        int write(std::shared_ptr<Work> consignor_work, char* str_buf, size_t str_len) override {
            auto work_out = std::make_shared<Work_NetTcpOut>(consignor_work, m_socket, str_buf, str_len);
            expect_ret_val(0 == m_net_io_worker->add_work(std::static_pointer_cast<Work_NetIo_Asio>(work_out)), -1);
            expect_ret_val(consignor_work->m_wp.wp_yield(0), -1);
            return consignor_work->m_wp.m_yield_param;
        }
        int read(std::shared_ptr<Work> consignor_work, char* recv_buf, size_t buf_sz, size_t& recv_data_sz) override {
            auto work_in = std::make_shared<Work_NetTcpIn>(consignor_work, m_socket, recv_buf, buf_sz);
            expect_ret_val(0 == m_net_io_worker->add_work(std::static_pointer_cast<Work_NetIo_Asio>(work_in)), -1);
            expect_ret_val(consignor_work->m_wp.wp_yield(0), -1);
            if (consignor_work->m_wp.m_yield_param < 0) {
                return -1;
            }
            recv_data_sz = consignor_work->m_wp.m_yield_param;
            return consignor_work->m_wp.m_yield_param;
        }
    private:
        Worker_NetIo* m_net_io_worker = nullptr;
        std::shared_ptr<tcp::socket> m_socket = nullptr;
    };
    class TcpSocketConnector_Asio : public TcpSocketConnector {
    public:
        explicit TcpSocketConnector_Asio(Worker_NetIo* net_io_worker)
        : TcpSocketConnector((Worker*)net_io_worker), m_net_io_worker(net_io_worker) {
            m_socket_to_server = std::make_shared<tcp::socket>(net_io_worker->m_io_ctx);
        }
        ~TcpSocketConnector_Asio() { m_socket_to_server->close(); }
        int connect(std::shared_ptr<Work> consignor_work, const std::string& addr_str, uint16_t port) override;
        int write(std::shared_ptr<Work> consignor_work, char* str_buf, size_t str_len) override;
        int read(std::shared_ptr<Work> consignor_work, char* recv_buf, size_t buf_sz, size_t& recv_data_sz) override;
        Worker_NetIo* m_net_io_worker = nullptr;
        std::shared_ptr<tcp::socket> m_socket_to_server = nullptr;
    };
    class TcpAcceptor_Asio : public TcpAcceptor {
    public:
        explicit TcpAcceptor_Asio(Worker_NetIo* net_io_worker)
        : TcpAcceptor((Worker*)net_io_worker), m_net_io_worker(net_io_worker) {
            m_acceptor = std::make_shared<tcp::acceptor>(m_net_io_worker->m_io_ctx);
        }
        int listen(const std::string& addr_str, uint16_t port) override {
            boost::system::error_code ec;

            // server endpoint
            boost::asio::ip::address addr = boost::asio::ip::make_address(addr_str, ec);
            check_ec_ret_val(ec, -1, "make_address");
            tcp::endpoint endpoint(addr, port);

            // acceptor
            boost::asio::ip::tcp::acceptor::reuse_address reuse_address_option(true);
            m_acceptor->open(endpoint.protocol(), ec);
            check_ec_ret_val(ec, -1, "acceptor open");
            m_acceptor->set_option(reuse_address_option, ec);
            check_ec_ret_val(ec, -1, "acceptor set_option");
            m_acceptor->bind(endpoint, ec);
            check_ec_ret_val(ec, -1, "acceptor bind");

            m_acceptor->listen(boost::asio::socket_base::max_listen_connections, ec);
            check_ec_ret_val(ec, -1, "acceptor listen");

            return 0;
        }
        std::shared_ptr<TcpSocketAb> accept(std::shared_ptr<Work> consignor_work) override {

            auto work_acceptor = std::make_shared<Work_NetTcpAccept>(consignor_work, m_acceptor);
            expect_ret_val(0 == m_net_io_worker->add_work(std::static_pointer_cast<Work_NetIo_Asio>(work_acceptor)),
                           nullptr);

            expect_ret_val(consignor_work->m_wp.wp_yield(0), nullptr);
            if (0 != consignor_work->m_wp.m_yield_param) {
                log_error("failed to accept!");
                return nullptr;
            }

            boost::system::error_code ec;
            std::shared_ptr<tcp::socket> client_socket = work_acceptor->m_socket_to_a_client;
            work_acceptor->m_socket_to_a_client.reset();

            tcp::endpoint remote_ep = client_socket->remote_endpoint(ec);
            check_ec_ret_val(ec, nullptr, "failed to get remote_endpoint");

            log_info("New client, ip: %s, port: %u",
                     remote_ep.address().to_string().c_str(),
                     remote_ep.port());
            std::string peer_addr(remote_ep.address().to_string()) ;
            return std::make_shared<TcpSocket_Asio>(m_net_io_worker,
                                                    client_socket,
                                                    peer_addr, remote_ep.port());
        }
    private:
        Worker_NetIo* m_net_io_worker = nullptr;
        std::shared_ptr<tcp::acceptor> m_acceptor = nullptr;
    };

    class Timer : public WorkUtils {
    public:
        explicit Timer(Worker* worker, std::shared_ptr<Work> consignor_work)
        : WorkUtils(worker), m_consignor_work(consignor_work), m_wp(consignor_work->m_wp) {}
        virtual int wait_until(unsigned ts_ms) = 0;
        virtual int wait_for(unsigned ts_ms) = 0;
        virtual int cancel() = 0;
    protected:
        std::shared_ptr<Work> m_consignor_work = nullptr;
        WorkingPoint& m_wp;
    };
    class Timer_Asio : public Timer {
    public:
        explicit Timer_Asio(Worker_NetIo* worker, std::shared_ptr<Work> consignor_work)
        : Timer((Worker*)worker, consignor_work), m_net_io_worker(worker) {
            m_timer = std::make_shared<boost::asio::deadline_timer>(worker->m_io_ctx);
        }
        int wait_until(unsigned ts_ms) override ;
        int wait_for(unsigned ts_ms) override ;
        int cancel() override ;
    private:
        Worker_NetIo* m_net_io_worker = nullptr;
        std::shared_ptr<boost::asio::deadline_timer> m_timer;
    };

    class tcp_socket {
    public:
        explicit tcp_socket(std::shared_ptr<TcpSocketAb> tcp_socket)
                : m_tcp_socket(tcp_socket)
        {}
        virtual int write_some(std::shared_ptr<Work> consignor_work, uint8_t* buf, size_t data_sz) {
            return m_tcp_socket->write(consignor_work, (char *)buf, data_sz);
        }
        virtual int read_some(std::shared_ptr<Work> consignor_work, uint8_t* buf, size_t buf_sz) {
            size_t recv_data_sz;
            int ret;
            ret = m_tcp_socket->read(consignor_work, (char *)buf, buf_sz, recv_data_sz);
            return ret;
        }
        virtual bool write(std::shared_ptr<Work> consignor_work, uint8_t* buf, size_t data_sz) {
            size_t remain_data_sz = data_sz;
            int ret;
            while (remain_data_sz > 0) {
                ret = write_some(consignor_work, buf + (data_sz - remain_data_sz), remain_data_sz);
                if (ret <= 0) {
                    log_error("write_some failed! ret: %d", ret);
                    return false;
                } else if (ret > remain_data_sz) {
                    log_error("write_some failed! ret: %d, remain_data_sz: %zu", ret, remain_data_sz);
                    return false;
                } else {
                    remain_data_sz -= ret;
                }
            }
            return true;
        }
        virtual bool read(std::shared_ptr<Work> consignor_work, uint8_t* buf, size_t data_sz) {
            size_t remain_data_sz = data_sz;
            int ret;
            while (remain_data_sz > 0) {
                ret = read_some(consignor_work, buf + (data_sz - remain_data_sz), remain_data_sz);
                if (ret <= 0) {
                    log_error("read_some failed! ret: %d", ret);
                    return false;
                } else if (ret > remain_data_sz) {
                    log_error("read_some failed! ret: %d, remain_data_sz: %zu", ret, remain_data_sz);
                    return false;
                } else {
                    remain_data_sz -= ret;
                }
            }
            return true;
        }
    private:
        std::shared_ptr<TcpSocketAb> m_tcp_socket;
    };

    class channel_ab : public std::enable_shared_from_this<channel_ab> {
    public:
        channel_ab(std::shared_ptr<Worker> main_worker,
                   bool is_initiator)
        : m_main_worker(main_worker),
          m_light_channel_map(std::make_shared<std::map<uint64_t, light_channel_info>>()),
          m_is_initiator(is_initiator)
        {}
        virtual bool send_msg(std::shared_ptr<Work> consignor_work, std::string& msg) = 0;
        virtual bool recv_msg(std::shared_ptr<Work> consignor_work, std::string& msg) = 0;

        bool get_chat_msg(std::string& msg, uint64_t& id_in_chat_msg, std::string& chat_msg) {
            size_t msg_size = msg.size();
            uint8_t* cur_pos = (uint8_t*)msg.data();
            uint64_t msg_type;
            expect_ret_val(msg_size >= sizeof(msg_type), false);
            msg_type = *((uint64_t*)cur_pos);
            cur_pos += sizeof(uint64_t);
            boost::endian::big_to_native_inplace(msg_type);
            if (0 != msg_type) {
                return false;
            }
            expect_ret_val(msg_size >= (sizeof(msg_type) + sizeof(id_in_chat_msg)), false);
            id_in_chat_msg = *((uint64_t*)cur_pos);
//            cur_pos += sizeof(uint64_t);
            boost::endian::big_to_native_inplace(id_in_chat_msg);
            chat_msg = msg.substr(sizeof(msg_type) + sizeof(id_in_chat_msg), std::string::npos);
            return true;
        }

        virtual bool send_text(std::shared_ptr<Work> consignor_work, uint64_t peer_id, std::string& text) {
            uint64_t msg_type = boost::endian::native_to_big((uint64_t)0);
            uint64_t peer_id_net = boost::endian::native_to_big(peer_id);
            std::string msg_type_str;
            std::string peer_id_str;
            std::string send_msg_str;
            msg_type_str.assign((const char*)(&msg_type), sizeof(msg_type));
            peer_id_str.assign((const char*)(&peer_id_net), sizeof(peer_id_net));
            send_msg_str = msg_type_str + peer_id_str + text;
            return send_msg(consignor_work, send_msg_str);
        }
        virtual bool recv_text(std::shared_ptr<Work> consignor_work, uint64_t& id_in_msg, std::string& text) {
            if (!recv_msg(consignor_work, text)) {
                return false;
            }
            uint64_t msg_type;
            expect_ret_val(text.size() >= (sizeof(msg_type) + sizeof(id_in_msg)), false);
            msg_type = *((uint64_t*)text.data());
            id_in_msg = *(((uint64_t*)text.data()) + 1);
            boost::endian::big_to_native_inplace(msg_type);
            boost::endian::big_to_native_inplace(id_in_msg);
            if (0 != msg_type) {
                log_error("this is not text message! type: %llu", msg_type);
                return false;
            }
            text = text.substr(sizeof(msg_type) + sizeof(id_in_msg), text.size() - sizeof(msg_type));
            return true;
        }

//        virtual bool add_light_channel(uint64_t& out_light_ch_id) {
//            out_light_ch_id = generate_light_ch_id();
//            auto ret = m_light_channel_map->emplace(out_light_ch_id, light_channel_info{});
//            if (ret.second) {
//                log_error("light channel map insert failed!");
//                return false;
//            }
//            return true;
//        }
        virtual bool del_light_channel(uint64_t light_ch_id) {
            m_light_channel_map->erase(light_ch_id);
            return true;
        }

        virtual bool add_light_channel_msg_handler(
                uint64_t light_ch_id,
                std::shared_ptr<Work> light_channel_consignor_work) {
            auto ret = m_light_channel_map->find(light_ch_id);
            if (ret == m_light_channel_map->end()) {
                return false;
            }
            ret->second.light_channel_consignor_work = light_channel_consignor_work;
            return true;
        }
        virtual bool del_light_channel_msg_handler(
                uint64_t light_ch_id) {
            auto ret = m_light_channel_map->find(light_ch_id);
            if (ret == m_light_channel_map->end()) {
                return false;
            }
            ret->second.light_channel_consignor_work.reset();
            return true;
        }
        virtual bool get_light_channel_msg(uint64_t light_ch_id, std::string& msg) {
            auto ret = m_light_channel_map->find(light_ch_id);
            if (ret == m_light_channel_map->end()) {
                return false;
            }
            if (ret->second.msg_q->empty()) {
                return false;
            }
            msg = ret->second.msg_q->front();
            ret->second.msg_q->pop();
            return true;
        }
    protected:
        uint64_t generate_light_ch_id() {
            uint64_t id = light_ch_id_base + 1;
            if (m_is_initiator) {
                id |= (uint64_t)1 << 63U;
            } else {
                id &= ~((uint64_t)1 << 63U);
            }
            light_ch_id_base++;
            return id;
        }
        virtual bool deal_with_light_channel_msg(uint64_t light_ch_id, std::string& msg) {
            auto ret = m_light_channel_map->find(light_ch_id);
            if (ret == m_light_channel_map->end()) {
                auto insert_ret = m_light_channel_map->emplace(light_ch_id, light_channel_info{});
                if (insert_ret.second) {
                    log_error("light channel map insert failed!");
                    return false;
                }
                ret = insert_ret.first;
            }
            light_channel_info& ch_info = ret->second;
            ch_info.msg_q->push(msg);
            if (ch_info.light_channel_consignor_work) {
                m_main_worker->add_work(new WorkWrap(ch_info.light_channel_consignor_work));
                ch_info.light_channel_consignor_work.reset();
            }
            return true;
        }
        struct light_channel_info {
            light_channel_info()
            : msg_q(std::make_shared<std::queue<std::string>>())
            {}
            std::shared_ptr<Work> light_channel_consignor_work;
            std::shared_ptr<std::queue<std::string>> msg_q;
        };
        std::shared_ptr<std::map<uint64_t, light_channel_info>> m_light_channel_map;
        std::shared_ptr<Worker> m_main_worker;
        bool m_is_initiator;
        uint64_t light_ch_id_base = 0;
    };

    // message format example:
    // msg: <uint64 msg size><uint64 msg type><msg body>
    //      <uint64 msg size>: all data size after <uint64 msg size>
    //      <uint64 msg type>: 0, chat message; 1 light channel msg
    // chat message <msg body>: <uint64 peer id><text>
    // light channel <msg body>: <uint64 light channel id><uint64 msg type><msg body>
    class light_channel_ab : public channel_ab {
    public:
        light_channel_ab(std::shared_ptr<Worker> main_worker,
                         std::shared_ptr<channel_ab> main_channel,
                         uint64_t light_channel_id,
                         bool is_initiator)
                : channel_ab(main_worker, is_initiator),
                  m_main_channel(main_channel),
                  m_light_channel_id(light_channel_id) {}
        virtual bool send_msg(std::shared_ptr<Work> consignor_work, std::string& msg) {
            uint64_t msg_type = boost::endian::native_to_big((uint64_t)1);
            uint64_t channel_id = boost::endian::native_to_big(m_light_channel_id);
            std::string msg_type_str;
            std::string channel_id_str;
            std::string channel_msg;
            msg_type_str.assign((const char*)(&msg_type), sizeof(msg_type));
            channel_id_str.assign((const char*)(&channel_id), sizeof(channel_id));
            channel_msg = msg_type_str + channel_id_str + msg;
            return m_main_channel->send_msg(consignor_work, channel_msg);
        }
        virtual bool recv_msg(std::shared_ptr<Work> consignor_work, std::string& msg) {
            if (m_main_channel->get_light_channel_msg(m_light_channel_id, msg)) {
                return true;
            }
            m_main_channel->add_light_channel_msg_handler(m_light_channel_id, consignor_work);
            expect_ret_val(consignor_work->m_wp.wp_yield(0), false);
            if (0 != consignor_work->m_wp.m_yield_param) {
                return false;
            }
            return m_main_channel->get_light_channel_msg(m_light_channel_id, msg);
        }
    protected:
        std::shared_ptr<channel_ab> m_main_channel;
        uint64_t m_light_channel_id;
    };

    class channel_builder_ab {
    public:
        // for client
        virtual std::shared_ptr<channel_ab> connect(std::shared_ptr<Work> consignor_work) {
            log_error("should not call me!");
            return nullptr;
        };
        // for server
        virtual bool listen() {
            log_error("should not call me!");
            return false;
        }
        virtual std::shared_ptr<channel_ab> accept(std::shared_ptr<Work> consignor_work) {
            log_error("should not call me!");
            return nullptr;
        }
    };

    class server_deal_req_work_ab : public Work {
    public:
        server_deal_req_work_ab(std::shared_ptr<std::string> req_msg,
                                std::shared_ptr<channel_ab> server_channel)
                : m_req_msg(req_msg), m_server_channel(server_channel) {}
        void do_work() override {
            if (true /* req_msg is xxx */) {
                /* do something */
                log_info("req msg: %s", m_req_msg->c_str());
                std::string res_msg = "response";
                m_server_channel->send_msg(shared_from_this(), res_msg);
            }
        }
    protected:
        std::shared_ptr<std::string> m_req_msg;
        std::shared_ptr<channel_ab> m_server_channel;
    };

    class server_ab {
    public:
        server_ab(std::shared_ptr<channel_ab> channel,
                  std::shared_ptr<Work> consignor_work,
                  std::shared_ptr<Worker> main_worker)
                : m_channel(channel),
                  m_consignor_work(consignor_work),
                  m_main_worker(main_worker) {}
        virtual void run() {
            while (true) {
                auto req_msg = std::make_shared<std::string>();
                m_channel->recv_msg(m_consignor_work, *req_msg);
                auto deal_req_work = get_server_deal_req_work(req_msg);
                m_main_worker->add_work(new WorkWrap(deal_req_work));
            }
        }
        virtual std::shared_ptr<server_deal_req_work_ab> get_server_deal_req_work(
                std::shared_ptr<std::string> req_msg) {
            return std::make_shared<server_deal_req_work_ab>(req_msg, m_channel);
        }
    protected:
        std::shared_ptr<channel_ab> m_channel;
        std::shared_ptr<Work> m_consignor_work;
        std::shared_ptr<Worker> m_main_worker;
    };
}

#endif //CPP_TOOLKIT_MOD_WORKUTILS_H
