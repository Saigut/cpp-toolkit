#include "app_im/app_im.h"

#include <mod_common/log.h>

#include "app_im_client.h"
#include "app_im_server.h"


// im_tcp_socket_impl
int im_tcp_socket_impl::write_some(std::shared_ptr<Work> consignor_work, uint8_t* buf, size_t data_sz) {
    return m_tcp_socket->write(consignor_work, (char *)buf, data_sz);
}
int im_tcp_socket_impl::read_some(std::shared_ptr<Work> consignor_work, uint8_t* buf, size_t buf_sz) {
    size_t recv_data_sz;
    int ret;
    ret = m_tcp_socket->read(consignor_work, (char *)buf, buf_sz, recv_data_sz);
    return ret;
}

// im_channel_impl
bool im_channel_impl::send_msg(std::shared_ptr<Work> consignor_work, uint64_t id, std::string& msg) {
    uint64_t msg_size_net_byte = boost::endian::native_to_big(sizeof(uint64_t) + msg.size());
    uint64_t id_net_byte = boost::endian::native_to_big(id);
    expect_ret_val(m_tcp->write(consignor_work, (uint8_t*)(&msg_size_net_byte), sizeof(uint64_t)), false);
    expect_ret_val(m_tcp->write(consignor_work, (uint8_t*)(&id_net_byte), sizeof(uint64_t)), false);
    expect_ret_val(m_tcp->write(consignor_work, (uint8_t*)(msg.data()), msg.size()), false);
    return true;
}
bool im_channel_impl::recv_msg(std::shared_ptr<Work> consignor_work, uint64_t& id, std::string& msg) {
    uint64_t msg_size;
    uint64_t read_id;

    expect_ret_val(m_tcp->read(consignor_work, (uint8_t*)(&msg_size), sizeof(uint64_t)), false);
    boost::endian::big_to_native_inplace(msg_size);
    expect_ret_val(msg_size >= sizeof(uint64_t), false);

    expect_ret_val(m_tcp->read(consignor_work, (uint8_t*)(&read_id), sizeof(uint64_t)), false);
    boost::endian::big_to_native_inplace(read_id);

    id = read_id;
    if (msg_size > sizeof(uint64_t)) {
        msg.reserve(msg_size - sizeof(uint64_t) + 1);
        msg.resize(msg_size - sizeof(uint64_t));
        expect_ret_val(m_tcp->read(consignor_work, (uint8_t*)(msg.data()), msg_size - sizeof(uint64_t)), false);
        ((uint8_t*)(msg.data()))[msg_size - sizeof(uint64_t)] = 0;
    }
    return true;
}

// im_channel_builder_impl
std::shared_ptr<im_channel_impl> im_channel_builder_impl::connect(std::shared_ptr<Work> consignor_work,
                                                                  uint64_t my_id,
                                                                  const std::string& addr_str,
                                                                  uint16_t port) {
    auto connector_asio = std::make_shared<::WorkUtils::TcpSocketConnector_Asio>((Worker_NetIo*)m_worker);
    expect_ret_val(0 == connector_asio->connect(consignor_work, addr_str, port), nullptr);
    auto channel = std::make_shared<im_channel_impl>(std::make_shared<im_tcp_socket_impl>(connector_asio));
    uint64_t id = 0;
    std::string msg;
    uint64_t sending_my_id = my_id;
    boost::endian::native_to_big_inplace(sending_my_id);
    msg.assign((const char*)(&sending_my_id), sizeof(sending_my_id));
    channel->send_msg(consignor_work, id, msg);
    return channel;
}
bool im_channel_builder_impl::listen(const std::string& local_addr_str, uint16_t local_port) {
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
std::shared_ptr<im_channel_impl> im_channel_builder_impl::accept(std::shared_ptr<Work> consignor_work, uint64_t& peer_id) {
    if (!acceptor_asio) {
        return nullptr;
    }
    std::shared_ptr<::WorkUtils::TcpSocketAb> socket_asio = acceptor_asio->accept(consignor_work);
    auto channel = std::make_shared<im_channel_impl>(std::make_shared<im_tcp_socket_impl>(socket_asio));
    uint64_t id;
    std::string msg;
    channel->recv_msg(consignor_work, id, msg);
    peer_id = *((uint64_t*)msg.data());
    boost::endian::big_to_native_inplace(peer_id);
    return channel;
}

// im2_channel
std::shared_ptr<WorkUtils::light_channel_ab> im2_channel::get_light_channel_from_msg(
        std::string &msg) {
    size_t msg_size = msg.size();
    uint8_t* cur_pos = (uint8_t*)msg.data();
    uint64_t msg_type;
    uint64_t light_channel_id;
    expect_ret_val(msg_size >= (sizeof(msg_type) + sizeof(light_channel_id)), nullptr);
    msg_type = *((uint64_t*)cur_pos);
    cur_pos += sizeof(uint64_t);
    boost::endian::big_to_native_inplace(msg_type);
    light_channel_id = *((uint64_t*)cur_pos);
//    cur_pos += sizeof(uint64_t);
    boost::endian::big_to_native_inplace(light_channel_id);
    if (1 != msg_type) {
        return nullptr;
    }

    auto light_channel_info_itr = m_light_channel_map->find(light_channel_id);
    if (light_channel_info_itr == m_light_channel_map->end()) {
        auto ret_insert = m_light_channel_map->emplace(light_channel_id, light_channel_info{});
        if (!ret_insert.second) {
            log_error("light channel map failed to insert!");
            return nullptr;
        }
        light_channel_info_itr = ret_insert.first;
    } else {
        if (light_channel_info_itr->second.light_channel) {
            return nullptr;
        }
    }

    uint64_t flag = (uint64_t)1 << 63;
    bool is_peer_channel_initiator = (0 != (light_channel_id & flag)) ? true : false;
    if (m_is_initiator == is_peer_channel_initiator) {
        return nullptr;
    }
    light_channel_info_itr->second.light_channel =
            std::make_shared<im2_light_channel>(m_main_worker,
                                                shared_from_this(),
                                                light_channel_id,
                                                m_is_initiator);
    return light_channel_info_itr->second.light_channel;
}

// im2_channel_builder
std::shared_ptr<im2_channel> im2_channel_builder::connect(std::shared_ptr<Work> consignor_work,
                                                                  uint64_t my_id,
                                                                  const std::string& addr_str,
                                                                  uint16_t port) {
    auto connector_asio = std::make_shared<::WorkUtils::TcpSocketConnector_Asio>(&(*m_io_worker));
    expect_ret_val(0 == connector_asio->connect(consignor_work, addr_str, port), nullptr);
    auto channel = std::make_shared<im2_channel>(connector_asio, m_main_worker, true);
    uint64_t id = 0;
    std::string msg;
    uint64_t sending_my_id = my_id;
    boost::endian::native_to_big_inplace(sending_my_id);
    msg.assign((const char*)(&sending_my_id), sizeof(sending_my_id));
    channel->send_text(consignor_work, id, msg);
    return channel;
}
bool im2_channel_builder::listen(const std::string& local_addr_str, uint16_t local_port) {
    if (acceptor_asio) {
        return true;
    }
    acceptor_asio = std::make_shared<::WorkUtils::TcpAcceptor_Asio>(&(*m_io_worker));
    if (0 != acceptor_asio->listen(local_addr_str, local_port)) {
        acceptor_asio.reset();
        return false;
    } else {
        return true;
    }
}
std::shared_ptr<im2_channel> im2_channel_builder::accept(std::shared_ptr<Work> consignor_work, uint64_t& peer_id) {
    if (!acceptor_asio) {
        return nullptr;
    }
    std::shared_ptr<::WorkUtils::TcpSocketAb> socket_asio = acceptor_asio->accept(consignor_work);
    auto channel = std::make_shared<im2_channel>(socket_asio, m_main_worker, false);
    uint64_t id;
    std::string msg;
    channel->recv_text(consignor_work, id, msg);
    peer_id = *((uint64_t*)msg.data());
    boost::endian::big_to_native_inplace(peer_id);
    return channel;
}

// im2_channel_recv_work
void im2_channel_recv_work::do_work() {
    while (true) {
        std::string msg;
        if (!m_channel->recv_msg(shared_from_this(), msg)) {
            log_error("channel failed to receive message!");
            break;
        }

        uint64_t id_in_msg;
        std::string chat_msg;
        if (m_channel->get_chat_msg(msg, id_in_msg, chat_msg)) {
            log_info("got message: %s", chat_msg.c_str());
        } else {
            auto light_channel = m_channel->get_light_channel_from_msg(msg);
            if (light_channel) {
                m_main_worker->add_work(new WorkWrap(
                        std::make_shared<im2_light_channel_server_work>(
                                light_channel, 11111)));
            }
            m_channel->deal_with_msg(msg);
        }
    }
}

// im2_channel_send_work
void im2_channel_send_work::do_work() {
    auto light_channel = std::make_shared<im2_light_channel>(
            m_main_worker_sp,
            m_channel,
            m_channel->generate_light_ch_id(),
            true);
    m_main_worker->add_work(new WorkWrap(
            std::make_shared<im2_light_channel_send_work>(
                    light_channel,
                    m_main_worker_sp,
                    m_io_worker,
                    m_my_id)));
    m_main_worker->add_work(new WorkWrap(
            std::make_shared<im2_light_channel_server_work>(
                    light_channel,
                    m_my_id)));
}

// im2_light_channel_send_work
void im2_light_channel_send_work::do_work() {
    std::string msg = "msg from: " + std::to_string(m_my_id);
    WorkUtils::Timer_Asio timer{&(*m_io_worker), shared_from_this()};
    while (true) {
        if (!m_channel->send_text(shared_from_this(), 11111, msg)) {
            log_error("failed to send message to server!");
            break;
        }
        // todo: here can recv_text for this light channel. and not block message in other channels
        timer.wait_for(5000);
    }
}

int app_im(int argc, char** argv)
{
    if (argc < 2) {
        log_error("wrong parameters.");
//        print_usage();
        return -1;
    }

    if (0 == strcmp(argv[1], "c")) {
//        return app_im_client_new(argc - 1, argv + 1);
        return app_im2_client(argc - 1, argv + 1);
    } else if (0 == strcmp(argv[1], "s")) {
//        return app_im_server_new(argc - 1, argv + 1);
        return app_im2_server(argc - 1, argv + 1);
    } else {
        log_error("wrong parameters.");
//        print_usage();
        return -1;
    }
}
