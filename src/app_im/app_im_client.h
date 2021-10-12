#ifndef CPP_TOOLKIT_APP_IM_CLIENT_H
#define CPP_TOOLKIT_APP_IM_CLIENT_H

#include <mod_queue/mod_queue.h>
#include <memory>
#include <cpt_im.grpc.pb.h>

#include <boost/endian.hpp>
#include <mod_common/expect.h>
#include <mod_worker/mod_work.h>
#include <mod_worker/mod_worker_netio.h>
#include <mod_worker/mod_workutils.h>

#ifdef __cplusplus
extern "C" {
#endif

// ab1
class im_client_ab1 {
public:
    virtual void send_msg(uint64_t id, std::string& msg) = 0;
    virtual void recv_msg(uint64_t& id, std::string& msg) = 0;
    uint64_t m_id;
};


// ab2
class im_client_ab2 {
public:
    virtual void send_msg(uint64_t id, std::string& msg) = 0;
    virtual void recv_msg(uint64_t& id, std::string& msg) = 0;
    uint64_t m_id;
    std::string m_server_addr;
};

class im_server_ab2 {
public:
    virtual void send_msg(uint64_t id, std::string& msg) = 0;
    virtual void recv_msg(uint64_t& id, std::string& msg) = 0;
};

// ab3
class im_client_ab3 {
public:
    virtual void send_msg(uint64_t id, std::string& msg) = 0;
    virtual void recv_msg(uint64_t& id, std::string& msg) = 0;

    virtual void work_send_msg() {
        std::string msg = "msg from: " + std::to_string(m_id);
        while (true) {
            send_msg(m_id, msg);
            std::this_thread::sleep_for (
                    std::chrono::seconds(5));
        }
    }

    virtual void work_recv_msg() {
        uint64_t id;
        std::string msg;
        recv_msg(id, msg);
        log_info("[msg:%u] %s", id, msg.c_str());
    }

    uint64_t m_id;
    std::string m_server_addr;
};

class im_server_ab3 {
public:
    virtual void send_msg(uint64_t id, std::string& msg) = 0;
    virtual void recv_msg(uint64_t& id, std::string& msg) = 0;

    virtual void work_recv_msg() {
        uint64_t id;
        std::string msg;
        recv_msg(id, msg);
        // if there is id
        // then send_msg(id, msg);
    }

//    virtual void work_send_msg() {
//
//    }
//
//    virtual void push_to_send_msg_queue(uint64_t id, std::string& msg) = 0;
//    virtual void pop_from_send_msg_queue(uint64_t& id, std::string& msg) = 0;
//
//    struct a_msg_t {
//        uint64_t id;
//        std::string msg;
//    };
//    std::queue<a_msg_t> m_send_msg_queue;
};

// ab4
class im_client_ab4 {
public:
    virtual void send_msg(uint64_t id, std::string& msg) = 0;
    virtual void recv_msg(uint64_t& id, std::string& msg) = 0;

    virtual void work_send_msg() {
        std::string msg = "msg from: " + std::to_string(m_id);
        while (true) {
            send_msg(m_id, msg);
            std::this_thread::sleep_for (
                    std::chrono::seconds(5));
        }
    }

    virtual void work_recv_msg() {
        uint64_t id;
        std::string msg;
        recv_msg(id, msg);
        log_info("[msg:%u] %s", msg.c_str());
    }

    uint64_t m_id;
    std::string m_server_addr;
    void* server_msg_channel;
};

class im_server_ab4 {
public:
    void send_msg(uint64_t id, std::string& msg) {
        void* channel;
        if (find_channel_of_id(id, channel)) {
            send_msg_to_channel(id, msg, channel);
        }
    }
    virtual void recv_msg(uint64_t& id, std::string& msg) = 0;
    virtual void send_msg_to_channel(uint64_t id, std::string& msg, void* channel) = 0;

    virtual void work_recv_msg() {
        uint64_t id;
        std::string msg;
        recv_msg(id, msg);
        // if there is id
        // then send_msg(id, msg);
        send_msg(id, msg);
    }

    bool find_channel_of_id(uint64_t id, void*& channel) {
        auto result = m_id_channel_map.find(id);
        if (result != m_id_channel_map.end()) {
            channel = result->second;
            return true;
        } else {
            return false;
        }
    }

    std::map<uint64_t, void*> m_id_channel_map;
};

// ab5
class im_channel_ab5 {
public:
    virtual void send_msg(uint64_t id, std::string& msg) = 0;
    virtual void recv_msg(uint64_t& id, std::string& msg) = 0;
};

class im_channel_builder_ab5 {
public:
    std::shared_ptr<im_channel_ab5> connect(void* peer_addr);
    std::shared_ptr<im_channel_ab5> accept(void* local_addr);
};

// ab6
class im_tcp_socket_ab6 {
public:
    virtual int write_some(std::shared_ptr<Work> consignor_work, uint8_t* buf, size_t data_sz) = 0;
    virtual int read_some(std::shared_ptr<Work> consignor_work, uint8_t* buf, size_t buf_sz) = 0;
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
    std::string m_peer_addr;
    uint16_t m_peer_port;
};

class im_tcp_acceptor_ab6 {
public:
    virtual bool listen(std::string& listen_addr, uint16_t listen_port) = 0;
    virtual im_tcp_socket_ab6* accept() = 0;

private:
    std::string m_listen_addr;
    uint16_t m_listen_port;
};

class im_channel_ab6 {
public:
    virtual bool send_msg(std::shared_ptr<Work> consignor_work, uint64_t id, std::string& msg) {
        uint64_t msg_size_net_byte = boost::endian::native_to_big(sizeof(uint64_t) + msg.size());
        uint64_t id_net_byte = boost::endian::native_to_big(id);
        expect_ret_val(m_tcp->write(consignor_work, (uint8_t*)(&msg_size_net_byte), sizeof(uint64_t)), false);
        expect_ret_val(m_tcp->write(consignor_work, (uint8_t*)(&id_net_byte), sizeof(uint64_t)), false);
        expect_ret_val(m_tcp->write(consignor_work, (uint8_t*)(msg.data()), msg.size()), false);
        return true;
    }
    virtual bool recv_msg(std::shared_ptr<Work> consignor_work, uint64_t& id, std::string& msg) {
        uint64_t msg_size;
        uint64_t read_id;

        expect_ret_val(m_tcp->read(consignor_work, (uint8_t*)(&msg_size), sizeof(uint64_t)), false);
        boost::endian::big_to_native_inplace(msg_size);
        expect_ret_val(msg_size >= sizeof(uint64_t), false);

        expect_ret_val(m_tcp->read(consignor_work, (uint8_t*)(&read_id), sizeof(uint64_t)), false);
        boost::endian::big_to_native_inplace(read_id);

        if (msg_size > sizeof(uint64_t)) {
            /// fixme!!
            expect_ret_val(m_tcp->read(consignor_work, (uint8_t*)(msg.data()), msg_size - sizeof(uint64_t)), false);
        }
        return true;
    }

    im_tcp_socket_ab6* m_tcp;
};

// impl
class im_tcp_socket_impl : public im_tcp_socket_ab6 {
public:
    explicit im_tcp_socket_impl(std::shared_ptr<WorkUtils::TcpSocketAb> tcp_socket)
    : m_tcp_socket(tcp_socket)
    {}
    int write_some(std::shared_ptr<Work> consignor_work, uint8_t* buf, size_t data_sz) override;
    int read_some(std::shared_ptr<Work> consignor_work, uint8_t* buf, size_t buf_sz) override;
private:
    std::shared_ptr<WorkUtils::TcpSocketAb> m_tcp_socket;
};

class im_channel_impl {
public:
    im_channel_impl(std::shared_ptr<im_tcp_socket_impl> tcp)
    : m_tcp(tcp) {}
    bool send_msg(std::shared_ptr<Work> consignor_work, uint64_t id, std::string& msg);
    bool recv_msg(std::shared_ptr<Work> consignor_work, uint64_t& id, std::string& msg);
private:
    std::shared_ptr<im_tcp_socket_impl> m_tcp;
};

class im_channel_builder_impl : public WorkUtils::WorkUtils {
public:
    explicit im_channel_builder_impl(Worker_NetIo* worker)
    : WorkUtils((Worker*)worker) {}
    std::shared_ptr<im_channel_impl> connect(std::shared_ptr<Work> consignor_work,
                                             uint64_t my_id,
                                             const std::string& addr_str,
                                             uint16_t port);

    bool listen(const std::string& local_addr_str, uint16_t local_port);
    std::shared_ptr<im_channel_impl> accept(std::shared_ptr<Work> consignor_work, uint64_t& peer_id);
private:
    std::shared_ptr<::WorkUtils::TcpAcceptor_Asio> acceptor_asio = nullptr;
};

class im_client_impl;
class ClientSendMsg : public Work {
public:
    explicit ClientSendMsg(std::shared_ptr<im_client_impl> client,
                           uint64_t my_id,
                           uint64_t peer_id,
                           Worker_NetIo* io_worker)
            : m_client(client),
              m_my_id(my_id),
              m_peer_id(peer_id),
              m_io_worker(io_worker) {}
    void do_work() override;
private:
    const uint64_t m_my_id;
    std::shared_ptr<im_client_impl> m_client;
    uint64_t m_peer_id;
    Worker_NetIo* m_io_worker;
};

class ClientRecvMsg : public Work {
public:
    explicit ClientRecvMsg(std::shared_ptr<im_client_impl> client)
            : m_client(client)
              {}
    void do_work() override;
private:
    std::shared_ptr<im_client_impl> m_client;
};

class im_client_impl {
public:
    im_client_impl(uint64_t my_id,
                   std::string &server_addr, uint16_t server_port)
            : m_id(my_id),
              m_server_addr(server_addr),
              m_server_port(server_port) {}

    bool connect(Worker_NetIo* worker, std::shared_ptr<Work> consignor_work);
    bool send_msg(std::shared_ptr<Work> consignor_work, uint64_t id, std::string& msg);
    bool recv_msg(std::shared_ptr<Work> consignor_work, uint64_t& id, std::string& msg);
    const uint64_t m_id;
private:
    std::string m_server_addr;
    uint16_t m_server_port;
    std::shared_ptr<im_channel_impl> server_msg_channel = nullptr;
};

class im_server_impl;
struct channel_info {
    channel_info(std::shared_ptr<im_channel_impl> _channel)
            : channel(_channel),
              msg_q(std::make_shared<std::queue<std::string>>())
    {}
    std::shared_ptr<im_channel_impl> channel;
    std::shared_ptr<std::queue<std::string>> msg_q;
};

// work: recv message from from a client
class RecvMsgFromClient : public Work {
public:
    explicit RecvMsgFromClient(uint64_t client_id,
                               std::shared_ptr<im_channel_impl> client_channel,
                               std::map<uint64_t, channel_info>& id_channel_map)
            : m_client_id(client_id),
              m_client_channel(client_channel),
              m_id_channel_map(id_channel_map) {}
    void do_work() override;
private:
    uint64_t m_client_id;
    std::shared_ptr<im_channel_impl> m_client_channel;
    std::map<uint64_t, channel_info>& m_id_channel_map;
};

// work: relay message to a client
class RelayMsgToClient : public Work {
public:
    explicit RelayMsgToClient(uint64_t client_id,
                              std::shared_ptr<im_channel_impl> client_channel,
                              std::shared_ptr<std::queue<std::string>> msg_q,
                              Worker_NetIo* io_worker)
            : m_client_id(client_id),
              m_client_channel(client_channel),
              m_msg_q(msg_q),
              m_io_worker(io_worker)
    {}
    void do_work() override;
private:
    uint64_t m_client_id;
    std::shared_ptr<im_channel_impl> m_client_channel;
    std::shared_ptr<std::queue<std::string>> m_msg_q;
    Worker_NetIo* m_io_worker = nullptr;
};

class im_server_impl {
public:
    im_server_impl(std::string& server_addr, uint16_t server_port,
                   Worker* main_worker,
                   Worker_NetIo* io_worker)
    : m_server_addr(server_addr),
      m_server_port(server_port),
      m_main_worker(main_worker),
      m_io_worker(io_worker),
      m_channel_builder(io_worker)
    {}
    bool listen();
    bool accept_channel(std::shared_ptr<Work> consignor_work);
private:
    Worker* m_main_worker;
    Worker_NetIo* m_io_worker;
    im_channel_builder_impl m_channel_builder;
    std::string m_server_addr;
    uint16_t m_server_port;
    std::map<uint64_t, channel_info> m_id_channel_map;
};

/// im2
// msg: <uint64 msg size><uint64 msg type><msg body>
//      <uint64 msg size>: all data size after <uint64 msg size>
//      <uint64 msg type>: 0, chat message; 1 light channel msg
// light channel <msg body>: <uint64 light channel id><uint64 msg type><msg body>
class im2_channel : public WorkUtils::channel_ab {
public:
    explicit im2_channel(std::shared_ptr<WorkUtils::TcpSocketAb> tcp_socket,
                         std::shared_ptr<Worker> main_worker)
            : channel_ab(main_worker),
              m_tcp(std::make_shared<WorkUtils::tcp_socket>(tcp_socket)) {}
    virtual bool send_msg(std::shared_ptr<Work> consignor_work, std::string& msg) override {
        uint64_t msg_size_net_byte = boost::endian::native_to_big(msg.size());
        expect_ret_val(m_tcp->write(consignor_work, (uint8_t*)(&msg_size_net_byte), sizeof(uint64_t)), false);
        expect_ret_val(m_tcp->write(consignor_work, (uint8_t*)(msg.data()), msg.size()), false);
        return true;
    }
    bool send_text(std::shared_ptr<Work> consignor_work, uint64_t peer_id, std::string& text) {
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
    virtual bool recv_msg(std::shared_ptr<Work> consignor_work, std::string& msg) override {
        uint64_t msg_size;
        uint64_t msg_type;

        expect_ret_val(m_tcp->read(consignor_work, (uint8_t*)(&msg_size), sizeof(msg_size)), false);
        boost::endian::big_to_native_inplace(msg_size);
        // check for msg type
        expect_ret_val(msg_size >= sizeof(msg_type), false);

        expect_ret_val(m_tcp->read(consignor_work, (uint8_t*)(&msg_type), sizeof(msg_type)), false);
        boost::endian::big_to_native_inplace(msg_type);

        switch (msg_type) {
            case 0: {
                msg.reserve(msg_size - sizeof(msg_type) + 1);
                msg.resize(msg_size - sizeof(uint64_t));
                expect_ret_val(m_tcp->read(consignor_work, (uint8_t*)(msg.data()), msg_size - sizeof(msg_type)), false);
                ((uint8_t*)(msg.data()))[msg_size - sizeof(uint64_t)] = 0;
                break;
            }
            case 1: {
                uint64_t light_channel_id;
                std::string light_channel_msg;
                expect_ret_val(msg_size >= (sizeof(msg_type) + sizeof(light_channel_id)), false);
                expect_ret_val(m_tcp->read(consignor_work, (uint8_t*)(&light_channel_id), sizeof(light_channel_id)), false);
                boost::endian::big_to_native_inplace(light_channel_id);
                light_channel_msg.reserve(msg_size - sizeof(msg_type) - sizeof(light_channel_id) + 1);
                light_channel_msg.resize(msg_size - sizeof(msg_type) - sizeof(light_channel_id));
                expect_ret_val(m_tcp->read(consignor_work, (uint8_t*)(light_channel_msg.data()), light_channel_msg.size()), false);
                expect_ret_val(deal_with_light_channel_msg(light_channel_id, light_channel_msg), false);
                msg.clear();
                break;
            }
            default: {
                log_error("Invalid message type: %llu", msg_type);
                return false;
            }
        }
        return true;
    }
    bool recv_text(std::shared_ptr<Work> consignor_work, uint64_t& id_in_msg, std::string& text) {
        uint64_t msg_size;
        uint64_t msg_type;
        uint64_t read_id;

        expect_ret_val(m_tcp->read(consignor_work, (uint8_t*)(&msg_size), sizeof(uint64_t)), false);
        boost::endian::big_to_native_inplace(msg_size);
        expect_ret_val(msg_size >= sizeof(msg_type), false);

        expect_ret_val(m_tcp->read(consignor_work, (uint8_t*)(&msg_type), sizeof(msg_type)), false);
        boost::endian::big_to_native_inplace(msg_type);

        if (0 != msg_type) {
            log_error("Unexpected message type: %llu!", msg_type);
            return false;
        }

        size_t msg_header_sz = sizeof(msg_type) + sizeof(read_id);
        expect_ret_val(msg_size >= msg_header_sz, false);
        expect_ret_val(m_tcp->read(consignor_work, (uint8_t*)(&read_id), sizeof(uint64_t)), false);
        boost::endian::big_to_native_inplace(read_id);

        id_in_msg = read_id;
        if (msg_size > msg_header_sz) {
            text.reserve(msg_size - msg_header_sz + 1);
            text.resize(msg_size - msg_header_sz);
            expect_ret_val(m_tcp->read(consignor_work, (uint8_t*)(text.data()), text.size()), false);
            ((uint8_t*)(text.data()))[msg_size - sizeof(uint64_t)] = 0;
        }
        return true;
    }
private:
    std::shared_ptr<WorkUtils::tcp_socket> m_tcp;
};

// Fixme: Should prevent same light channel id in two ends of main_channel!
class im2_light_channel : public WorkUtils::light_channel_ab {
public:
    im2_light_channel(std::shared_ptr<Worker> main_worker,
                      std::shared_ptr<WorkUtils::channel_ab> main_channel,
                      uint64_t light_channel_id,
                      uint64_t peer_id)
            : WorkUtils::light_channel_ab(main_worker,
                                          main_channel,
                                          light_channel_id),
              m_peer_id(peer_id) {}
    bool send_text(std::shared_ptr<Work> consignor_work, std::string& text) {
        uint64_t msg_type = boost::endian::native_to_big((uint64_t)0);
        uint64_t peer_id_net = boost::endian::native_to_big(m_peer_id);
        std::string msg_type_str;
        std::string peer_id_str;
        std::string send_msg_str;
        msg_type_str.assign((const char*)(&msg_type), sizeof(msg_type));
        peer_id_str.assign((const char*)(&peer_id_net), sizeof(peer_id_net));
        send_msg_str = msg_type_str + peer_id_str + text;
        return send_msg(consignor_work, send_msg_str);
    }
    bool recv_text(std::shared_ptr<Work> consignor_work, std::string& text) {
        if (!recv_msg(consignor_work, text)) {
            return false;
        }
        uint64_t msg_type;
        uint64_t peer_id;
        expect_ret_val(text.size() >= (sizeof(msg_type) + sizeof(peer_id)), false);
        msg_type = *((uint64_t*)text.data());
        peer_id = *(((uint64_t*)text.data()) + 1);
        boost::endian::big_to_native_inplace(msg_type);
        boost::endian::big_to_native_inplace(peer_id);
        if (0 != msg_type) {
            log_warn("this is not text message! type: %llu", msg_type);
        } else {
            text = text.substr(sizeof(msg_type) + sizeof(peer_id), text.size() - sizeof(msg_type));
        }
        return true;
    }
private:
    uint64_t m_peer_id;
};

class im2_channel_builder : public WorkUtils::channel_builder_ab {
public:
    explicit im2_channel_builder(std::shared_ptr<Worker> main_worker,
                                 std::shared_ptr<Worker_NetIo> io_worker)
            : m_main_worker(main_worker),
              m_io_worker(io_worker) {}
    std::shared_ptr<im2_channel> connect(std::shared_ptr<Work> consignor_work,
                                             uint64_t my_id,
                                             const std::string& addr_str,
                                             uint16_t port);

    bool listen(const std::string& local_addr_str, uint16_t local_port);
    std::shared_ptr<im2_channel> accept(std::shared_ptr<Work> consignor_work, uint64_t& peer_id);
private:
    std::shared_ptr<Worker> m_main_worker;
    std::shared_ptr<Worker_NetIo> m_io_worker;
    std::shared_ptr<::WorkUtils::TcpAcceptor_Asio> acceptor_asio = nullptr;
};

/// old
class im_client {
public:
    im_client(std::string& list_port,
              uint64_t user_id,
              uint64_t peer_user_id)
    :m_list_port(list_port),
    m_user_id(user_id),
    m_peer_user_id(peer_user_id),
    r_queue(),
    s_queue() {}
    int init();
    void start();
    int send_msg(const std::string& msg);
    int send_hb();
    void grpc_receive();
    void grpc_send();
    void send_msg_thread();

private:
    int deal_with_msg(const ::cpt_im::ClientIntfReq& req);

    uint64_t m_user_id;
    uint64_t m_peer_user_id;

    friend class CptImClientServiceImpl;
    cpt_queue<::cpt_im::ClientIntfReq> r_queue;
    cpt_queue<::cpt_im::ServerIntfReq> s_queue;

    std::string m_list_port;
};

int app_im_client(int argc, char** argv);
int app_im_client_new(int argc, char** argv);

#ifdef __cplusplus
}
#endif

#endif //CPP_TOOLKIT_APP_IM_CLIENT_H
