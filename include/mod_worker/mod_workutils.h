#ifndef CPP_TOOLKIT_MOD_WORKUTILS_H
#define CPP_TOOLKIT_MOD_WORKUTILS_H

#include <mod_worker/mod_work.h>
#include <mod_worker/mod_worker_netio.h>

#include <mod_common/expect.h>
#include <boost/asio/deadline_timer.hpp>
#include <boost/endian.hpp>

namespace WorkUtils {
    class WorkCoCbs {
    public:
        WorkCoCbs(std::function<void()> work_yield,
                  std::function<void()> work_back)
                : m_work_yield(work_yield),
                  m_work_back(work_back) {}
        std::function<void()> m_work_yield;
        std::function<void()> m_work_back;
    };
    class UserUtilOri {
    public:
//        void xxxxx(xxx, xxx, WorkCoCbs);
    };
    class UserUtil {
    public:
//        void xxxxx(xxx, xxx) {
//            m_user_util.xxxxx(xxx, xxx, m_co_cbs);
//        }
        UserUtilOri m_user_util;
        WorkCoCbs m_co_cbs;
    };
    class WorkUtilsInner {
    public:

    };
    class WorkUtils {
    public:
        explicit WorkUtils(WorkCoCbs& co_cbs)
                : m_co_cbs(co_cbs) {}
        explicit WorkUtils(std::function<void()> work_yield,
                           std::function<void()> work_back)
                : m_co_cbs(work_yield, work_back) {}
    protected:
        WorkCoCbs m_co_cbs;
    };

    class TcpSocketAb_Real {
    public:
        virtual int write(char* str_buf, size_t str_len, WorkCoCbs& co_cbs) {
            log_error("Should not call this!");
            return -1;
        }
        virtual int read(char* recv_buf, size_t buf_sz,size_t& recv_data_sz,
                         WorkCoCbs& co_cbs) {
            log_error("Should not call this!");
            return -1;
        }
    };

    class TcpSocketAb : public WorkUtils {
    public:
        explicit TcpSocketAb(std::shared_ptr<TcpSocketAb_Real> socket_real,
                             WorkCoCbs& co_cbs)
                : m_socket_real(socket_real),
                  WorkUtils(co_cbs) {}
        virtual int write(char* str_buf, size_t str_len) {
            return m_socket_real->write(str_buf, str_len, m_co_cbs);
        }
        virtual int read(char* recv_buf, size_t buf_sz, size_t& recv_data_sz) {
            return m_socket_real->read(recv_buf, buf_sz, recv_data_sz, m_co_cbs);
        }
        std::shared_ptr<TcpSocketAb_Real> m_socket_real;
    };

    class TcpSocketConnector : public TcpSocketAb {
    public:
        explicit TcpSocketConnector(std::function<void()> work_yield,
                                    std::function<void()> work_back)
                                    : TcpSocketAb(work_yield, work_back) {}
        virtual int connect(const std::string& addr_str, uint16_t port) = 0;
    };
    class TcpAcceptor {
    public:
        explicit TcpAcceptor(std::function<void()> work_yield,
                             std::function<void()> work_back)
                : m_work_yield(work_yield), m_work_back(work_back) {}
        virtual int listen(const std::string& addr_str, uint16_t port) = 0;
        virtual std::shared_ptr<TcpSocketAb> accept() = 0;
        std::function<void()> m_work_yield;
        std::function<void()> m_work_back;
    };
    class TcpSocket_Asio : public TcpSocketAb {
    public:
        explicit TcpSocket_Asio(std::function<void()> work_yield,
                                std::function<void()> work_back,
                                std::shared_ptr<tcp::socket> socket,
                                std::string& peer_addr,
                                uint16_t peer_port)
        : TcpSocketAb(work_yield, work_back),
        m_socket(socket)
        {}
        int write(char* str_buf, size_t str_len) override {
            int ret;
            auto work_back = m_work_back;
            boost::asio::mutable_buffer out_buf(str_buf, str_len);
            m_socket->async_write_some(out_buf,
                                       [&ret, work_back](const boost::system::error_code& ec,
                                                         std::size_t write_b_num) {
                                           check_ec(ec, "write_some");
                                           ret = ec ? -1 : (int) write_b_num;
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
        int read(char* recv_buf, size_t buf_sz, size_t& recv_data_sz) override {
            int ret;
            auto work_back = m_work_back;
            boost::asio::mutable_buffer in_buf(recv_buf, buf_sz);
            m_socket->async_read_some(in_buf,
                                      [&ret, work_back](const boost::system::error_code& ec,
                                                        std::size_t read_b_num) {
                                          check_ec(ec, "read_some");
                                          ret = ec ? -1 : (int) read_b_num;
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
    private:
//        Worker_NetIo* m_net_io_worker = nullptr;
        std::shared_ptr<tcp::socket> m_socket = nullptr;
    };
    class TcpSocketConnector_Asio : public TcpSocketConnector,
            public std::enable_shared_from_this<TcpSocketConnector_Asio> {
    public:
        explicit TcpSocketConnector_Asio(std::function<void()> work_yield,
                                         std::function<void()> work_back)
        : TcpSocketConnector(work_yield, work_back) {
            m_socket_to_server = std::make_shared<tcp::socket>(m_io_ctx);
        }
        ~TcpSocketConnector_Asio() { m_socket_to_server->close(); }

        int connect(const std::string& addr_str, uint16_t port) override;
        int write(char* str_buf, size_t str_len) override;
        int read(char* recv_buf, size_t buf_sz, size_t& recv_data_sz) override;

    private:
        io_context m_io_ctx;
        std::shared_ptr<tcp::socket> m_socket_to_server = nullptr;
    };
    class TcpAcceptor_Asio : public TcpAcceptor {
    public:
        explicit TcpAcceptor_Asio(std::function<void()> work_yield,
                                  std::function<void()> work_back)
        : TcpAcceptor(work_yield, work_back) {
            m_acceptor = std::make_shared<tcp::acceptor>(m_io_ctx);
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
        std::shared_ptr<TcpSocketAb> accept() override {
            int ret;
            auto work_back = m_work_back;
            std::shared_ptr<tcp::socket> client_socket;
            m_acceptor->async_accept([&ret, work_back, &client_socket](const boost::system::error_code& ec,
                    tcp::socket peer) {
                check_ec(ec, "accept");
                if (!ec) {
                    client_socket = std::make_shared<tcp::socket>(std::move(peer));
                }
                ret = ec ? -1 : 0;
                if (work_back) {
                    work_back();
                }
            });
            m_work_yield();

            if (0 != ret) {
                log_error("failed to accept!");
                return nullptr;
            }

            boost::system::error_code ec;
            tcp::endpoint remote_ep = client_socket->remote_endpoint(ec);
            check_ec_ret_val(ec, nullptr, "failed to get remote_endpoint");

            log_info("New client, ip: %s, port: %u",
                     remote_ep.address().to_string().c_str(),
                     remote_ep.port());
            std::string peer_addr(remote_ep.address().to_string()) ;
            return std::make_shared<TcpSocket_Asio>(m_work_yield, m_work_back,
                                                    client_socket,
                                                    peer_addr, remote_ep.port());
        }
    private:
        io_context m_io_ctx;
        std::shared_ptr<tcp::acceptor> m_acceptor = nullptr;
    };

//    class Timer : public WorkUtils {
//    public:
//        explicit Timer(Worker* worker, std::shared_ptr<Work> consignor_work)
//        : WorkUtils(worker), m_consignor_work(consignor_work), m_wp(consignor_work->m_wp) {}
//        virtual int wait_until(unsigned ts_ms) = 0;
//        virtual int wait_for(unsigned ts_ms) = 0;
//        virtual int cancel() = 0;
//    protected:
//        std::shared_ptr<Work> m_consignor_work = nullptr;
//        WorkingPoint& m_wp;
//    };
//    class Timer_Asio : public Timer {
//    public:
//        explicit Timer_Asio(Worker_NetIo* worker, std::shared_ptr<Work> consignor_work)
//        : Timer((Worker*)worker, consignor_work), m_net_io_worker(worker) {
//            m_timer = std::make_shared<boost::asio::deadline_timer>(worker->m_io_ctx);
//        }
//        int wait_until(unsigned ts_ms) override ;
//        int wait_for(unsigned ts_ms) override ;
//        int cancel() override ;
//    private:
//        Worker_NetIo* m_net_io_worker = nullptr;
//        std::shared_ptr<boost::asio::deadline_timer> m_timer;
//    };

#if 0
    class tcp_socket {
    public:
        explicit tcp_socket(std::shared_ptr<TcpSocketAb> tcp_socket)
                : m_tcp_socket(tcp_socket)
        {}
        virtual int write_some(uint8_t* buf, size_t data_sz) {
            return m_tcp_socket->write((char *)buf, data_sz);
        }
        virtual int read_some(uint8_t* buf, size_t buf_sz) {
            size_t recv_data_sz;
            int ret;
            ret = m_tcp_socket->read((char *)buf, buf_sz, recv_data_sz);
            return ret;
        }
        virtual bool write(uint8_t* buf, size_t data_sz) {
            size_t remain_data_sz = data_sz;
            int ret;
            while (remain_data_sz > 0) {
                ret = write_some(buf + (data_sz - remain_data_sz), remain_data_sz);
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
        virtual bool read(uint8_t* buf, size_t data_sz) {
            size_t remain_data_sz = data_sz;
            int ret;
            while (remain_data_sz > 0) {
                ret = read_some(buf + (data_sz - remain_data_sz), remain_data_sz);
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

    class light_channel_ab;
    class channel_ab : public std::enable_shared_from_this<channel_ab> {
    public:
        channel_ab(std::function<void()> work_yield,
                   std::function<void()> work_back,
                   bool is_initiator)
        : m_work_yield(work_yield), m_work_back(work_back),
          m_light_channel_map(std::make_shared<std::map<uint64_t, light_channel_info>>()),
          m_is_initiator(is_initiator)
        {}
        virtual bool send_msg(std::string& msg) = 0;
        virtual bool recv_msg(std::string& msg) = 0;

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

        virtual bool send_text(uint64_t peer_id, std::string& text) {
            uint64_t msg_type = boost::endian::native_to_big((uint64_t)0);
            uint64_t peer_id_net = boost::endian::native_to_big(peer_id);
            std::string msg_type_str;
            std::string peer_id_str;
            std::string send_msg_str;
            msg_type_str.assign((const char*)(&msg_type), sizeof(msg_type));
            peer_id_str.assign((const char*)(&peer_id_net), sizeof(peer_id_net));
            send_msg_str = msg_type_str + peer_id_str + text;
            return send_msg(send_msg_str);
        }
        virtual bool recv_text(uint64_t& id_in_msg, std::string& text) {
            if (!recv_msg(text)) {
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
                std::function<void(int)> light_channel_handler) {
            auto ret = m_light_channel_map->find(light_ch_id);
            if (ret == m_light_channel_map->end()) {
                auto insert_ret = m_light_channel_map->emplace(light_ch_id, light_channel_info{});
                if (!insert_ret.second) {
                    log_error("light channel map insert failed!");
                    return false;
                }
                ret = insert_ret.first;
            }
            ret->second.light_channel_handler = light_channel_handler;
            return true;
        }
        virtual bool del_light_channel_msg_handler(
                uint64_t light_ch_id) {
            auto ret = m_light_channel_map->find(light_ch_id);
            if (ret == m_light_channel_map->end()) {
                return false;
            }
            ret->second.light_channel_handler = [](int){};
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
                if (!insert_ret.second) {
                    log_error("light channel map insert failed!");
                    return false;
                }
                ret = insert_ret.first;
            }
            light_channel_info& ch_info = ret->second;
            ch_info.msg_q->push(msg);
            if (ch_info.light_channel_handler) {
                ch_info.light_channel_handler(0);
            }
            return true;
        }
        struct light_channel_info {
            light_channel_info()
            : msg_q(std::make_shared<std::queue<std::string>>())
            {}
            std::shared_ptr<std::queue<std::string>> msg_q;
            std::shared_ptr<light_channel_ab> light_channel;
            std::function<void(int)> light_channel_handler;
        };
        std::function<void()> m_work_yield;
        std::function<void()> m_work_back;
        std::shared_ptr<std::map<uint64_t, light_channel_info>> m_light_channel_map;
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
        light_channel_ab(std::function<void()> work_yield,
                         std::function<void()> work_back,
                         std::shared_ptr<channel_ab> main_channel,
                         uint64_t light_channel_id,
                         bool is_initiator)
                : channel_ab(work_yield, work_back, is_initiator),
                  m_main_channel(main_channel),
                  m_light_channel_id(light_channel_id) {}
        virtual bool send_msg(std::string& msg) {
            uint64_t msg_type = boost::endian::native_to_big((uint64_t)1);
            uint64_t channel_id = boost::endian::native_to_big(m_light_channel_id);
            std::string msg_type_str;
            std::string channel_id_str;
            std::string channel_msg;
            msg_type_str.assign((const char*)(&msg_type), sizeof(msg_type));
            channel_id_str.assign((const char*)(&channel_id), sizeof(channel_id));
            channel_msg = msg_type_str + channel_id_str + msg;
            return m_main_channel->send_msg(channel_msg);
        }
        virtual bool recv_msg(std::string& msg) {
            if (m_main_channel->get_light_channel_msg(m_light_channel_id, msg)) {
                return true;
            }
            int ret;
            auto work_back = m_work_back;
            m_main_channel->add_light_channel_msg_handler(m_light_channel_id,
                                                          [&ret, work_back](
                                                                  int func_ret) {
                                                              ret = func_ret;
                                                              if (work_back) {
                                                                  work_back();
                                                              }
                                                          });
            m_work_yield();
            if (0 != ret) {
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
        virtual std::shared_ptr<channel_ab> connect() {
            log_error("should not call me!");
            return nullptr;
        };
        // for server
        virtual bool listen() {
            log_error("should not call me!");
            return false;
        }
        virtual std::shared_ptr<channel_ab> accept() {
            log_error("should not call me!");
            return nullptr;
        }
    };
}
#endfi

#endif //CPP_TOOLKIT_MOD_WORKUTILS_H
