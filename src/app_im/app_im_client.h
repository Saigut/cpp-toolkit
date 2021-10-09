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
                return false;
            } else if (ret > remain_data_sz) {
                return false;
            } else {
                remain_data_sz -= ret;
            }
        }
    }
    virtual bool read(std::shared_ptr<Work> consignor_work, uint8_t* buf, size_t data_sz) {
        size_t remain_data_sz = data_sz;
        int ret;
        while (remain_data_sz > 0) {
            ret = read_some(consignor_work, buf + (data_sz - remain_data_sz), remain_data_sz);
            if (ret <= 0) {
                return false;
            } else if (ret > remain_data_sz) {
                return false;
            } else {
                remain_data_sz -= ret;
            }
        }
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
    }

    im_tcp_socket_ab6* m_tcp;
};

// impl
class im_tcp_socket_impl : public im_tcp_socket_ab6 {
public:
    explicit im_tcp_socket_impl(WorkUtils::TcpSocket& tcp_socket)
    : m_tcp_socket(tcp_socket)
    {}
    int write_some(std::shared_ptr<Work> consignor_work, uint8_t* buf, size_t data_sz) override {
        return m_tcp_socket.write(consignor_work, (char *)buf, data_sz);
    }
    int read_some(std::shared_ptr<Work> consignor_work, uint8_t* buf, size_t buf_sz) override {
        size_t recv_data_sz;
        int ret;
        ret = m_tcp_socket.read(consignor_work, (char *)buf, buf_sz, recv_data_sz);
        return ret;
    }
private:
    WorkUtils::TcpSocket m_tcp_socket;
};

class im_channel_impl {
public:
    im_channel_impl(std::shared_ptr<im_tcp_socket_impl> tcp)
    : m_tcp(tcp) {}
    bool send_msg(std::shared_ptr<Work> consignor_work, uint64_t id, std::string& msg) {
        uint64_t msg_size_net_byte = boost::endian::native_to_big(sizeof(uint64_t) + msg.size());
        uint64_t id_net_byte = boost::endian::native_to_big(id);
        expect_ret_val(m_tcp->write(consignor_work, (uint8_t*)(&msg_size_net_byte), sizeof(uint64_t)), false);
        expect_ret_val(m_tcp->write(consignor_work, (uint8_t*)(&id_net_byte), sizeof(uint64_t)), false);
        expect_ret_val(m_tcp->write(consignor_work, (uint8_t*)(msg.data()), msg.size()), false);
    }
    bool recv_msg(std::shared_ptr<Work> consignor_work, uint64_t& id, std::string& msg) {
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
    }
    std::shared_ptr<im_tcp_socket_impl> m_tcp;
};

class im_channel_builder_impl : public WorkUtils::WorkUtils {
public:
    explicit im_channel_builder_impl(Worker_NetIo* worker)
    : WorkUtils(worker) {}

    std::shared_ptr<im_channel_impl> connect(std::shared_ptr<Work> consignor_work, uint64_t my_id, const std::string& addr_str, uint16_t port) {
        ::WorkUtils::TcpSocketConnector_Asio connector_asio{(Worker_NetIo*)m_worker};
        expect_ret_val(0 == connector_asio.connect(consignor_work, addr_str, port), nullptr);
        auto channel = std::make_shared<im_channel_impl>(std::make_shared<im_tcp_socket_impl>(connector_asio));
        /// Fixme: exchange information with peer
        uint64_t id = 0;
        std::string msg;
        msg.assign((const char*)(&my_id), sizeof(my_id));
        channel->send_msg(consignor_work, id, msg);
        return channel;
    }

    bool listen(const std::string& local_addr_str, uint16_t local_port) {
        if (acceptor_asio) {
            return true;
        }
        acceptor_asio = std::make_shared<::WorkUtils::TcpAcceptor_Asio>((Worker_NetIo*)m_worker);
        if (0 != acceptor_asio->listen(local_addr_str, local_port)) {
            acceptor_asio.reset();
            return false;
        } else {
            return true;
        }
    }
    std::shared_ptr<im_channel_impl> accept(std::shared_ptr<Work> consignor_work, uint64_t& peer_id) {
        if (!acceptor_asio) {
            return nullptr;
        }
        std::shared_ptr<::WorkUtils::TcpSocket> socket_asio = acceptor_asio->accept(consignor_work);
        auto channel = std::make_shared<im_channel_impl>(std::make_shared<im_tcp_socket_impl>(*socket_asio));
        /// Fixme: exchange information with peer
        uint64_t id;
        std::string msg;
        channel->recv_msg(consignor_work, id, msg);
        peer_id = *((uint64_t*)msg.data());
        boost::endian::big_to_native_inplace(peer_id);
        return channel;
    }

    std::shared_ptr<::WorkUtils::TcpAcceptor_Asio> acceptor_asio = nullptr;
};

class im_client_impl {
public:
    im_client_impl(uint64_t my_id,
                   std::string& server_addr, uint16_t server_port)
    : m_server_addr(server_addr), m_server_port(server_port)
    {}

    bool setup_channel(Worker_NetIo* worker, std::shared_ptr<Work> consignor_work) {
        im_channel_builder_impl channel_builder{worker};
        server_msg_channel = channel_builder.connect(consignor_work, m_id, m_server_addr, m_server_port);
        if (!server_msg_channel) {
            log_error("connect failed!");
            return false;
        } else {
            return true;
        }
    }

    void send_msg(std::shared_ptr<Work> consignor_work, uint64_t id, std::string& msg) {
        server_msg_channel->send_msg(consignor_work, id, msg);
    }
    void recv_msg(std::shared_ptr<Work> consignor_work, uint64_t& id, std::string& msg) {
        server_msg_channel->recv_msg(consignor_work, id, msg);
    }

    void work_send_msg(std::shared_ptr<Work> consignor_work) {
        std::string msg = "msg from: " + std::to_string(m_id);
        while (true) {
            send_msg(consignor_work, m_id, msg);
            std::this_thread::sleep_for (
                    std::chrono::seconds(5));
        }
    }

    void work_recv_msg(std::shared_ptr<Work> consignor_work) {
        uint64_t id;
        std::string msg;
        recv_msg(consignor_work, id, msg);
        log_info("[msg:%u] %s", msg.c_str());
    }

    uint64_t m_id;
    std::string m_server_addr;
    uint16_t m_server_port;
    std::shared_ptr<im_channel_impl> server_msg_channel = nullptr;
};

class im_server_impl {
public:
    im_server_impl(uint64_t my_id,
                   std::string& server_addr, uint16_t server_port)
    : m_server_addr(server_addr), m_server_port(server_port)
    {}

    void send_msg(std::shared_ptr<Work> consignor_work, uint64_t id, std::string& msg) {
        std::shared_ptr<im_channel_impl> channel = find_channel_of_id(id);
        if (channel) {
            send_msg_to_channel(consignor_work, id, msg, channel);
        }
    }
    void recv_msg(std::shared_ptr<Work> consignor_work, uint64_t& id, std::string& msg) {
        std::map<uint64_t, std::shared_ptr<im_channel_impl>>::iterator itr;
        for (itr = m_id_channel_map.begin();
             itr != m_id_channel_map.end();
             itr++) {
            itr->second->recv_msg(consignor_work, id, msg);
            // new work and then, send_msg(work, id, msg);
        }
    }
    void send_msg_to_channel(std::shared_ptr<Work> consignor_work,
                             uint64_t id, std::string& msg,
                             std::shared_ptr<im_channel_impl> channel) {
        channel->send_msg(consignor_work, id, msg);
    }

    bool work_accept_channel(Worker_NetIo* worker, std::shared_ptr<Work> consignor_work) {
        im_channel_builder_impl channel_builder{worker};
        uint64_t peer_id;
        std::shared_ptr<im_channel_impl> client_channel = channel_builder.accept(consignor_work, peer_id);
        if (!client_channel) {
            log_error("accept failed!");
            return false;
        } else {
            m_id_channel_map.emplace(peer_id, client_channel);
            return true;
        }
    }

    std::shared_ptr<im_channel_impl> find_channel_of_id(uint64_t id) {
        auto result = m_id_channel_map.find(id);
        if (result != m_id_channel_map.end()) {
            return result->second;
        } else {
            return nullptr;
        }
    }

    std::string m_server_addr;
    uint16_t m_server_port;
    std::map<uint64_t, std::shared_ptr<im_channel_impl>> m_id_channel_map;
};


class im_client_base {
public:
    virtual void send_msg(uint64_t id, std::string& msg) = 0;
    virtual void recv_msg(uint64_t& id, std::string& msg) = 0;
    uint64_t m_id;
};

class im_client2 : public im_client_base {
public:
    void send_msg(uint64_t id, std::string& msg) override;
    void recv_msg(uint64_t& id, std::string& msg) override;
    void run();
};

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

#ifdef __cplusplus
}
#endif

#endif //CPP_TOOLKIT_APP_IM_CLIENT_H
