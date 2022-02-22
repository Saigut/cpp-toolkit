#include "app_im/app_im.h"

#include <mod_common/log.h>

#include "app_im_client.h"
#include "app_im_server.h"

// im_tcp_socket_impl
//int im_tcp_socket_impl::write_some(std::shared_ptr<Work> consignor_work, uint8_t* buf, size_t data_sz) {
//    return m_tcp_socket->write(consignor_work, (char *)buf, data_sz);
//}
//int im_tcp_socket_impl::read_some(std::shared_ptr<Work> consignor_work, uint8_t* buf, size_t buf_sz) {
//    size_t recv_data_sz;
//    int ret;
//    ret = m_tcp_socket->read(consignor_work, (char *)buf, buf_sz, recv_data_sz);
//    return ret;
//}

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
            std::make_shared<im2_light_channel>(shared_from_this(),
                                                light_channel_id,
                                                m_is_initiator,
                                                this->m_tcp->m_co_cbs);
    return light_channel_info_itr->second.light_channel;
}

// im2_channel_builder
std::shared_ptr<im2_channel> im2_channel_builder::connect(uint64_t my_id,
                                                                  const std::string& addr_str,
                                                                  uint16_t port) {
    auto skt = m_socket_builder.connect(addr_str, port);
    expect_ret_val(skt, nullptr);
    auto channel = std::make_shared<im2_channel>(skt, true);
    uint64_t id = 0;
    std::string msg;
    uint64_t sending_my_id = my_id;
    boost::endian::native_to_big_inplace(sending_my_id);
    msg.assign((const char*)(&sending_my_id), sizeof(sending_my_id));
    channel->send_text(id, msg);
    return channel;
}
bool im2_channel_builder::listen(const std::string& local_addr_str, uint16_t local_port) {
    return m_socket_builder.listen(local_addr_str, local_port);
}
std::shared_ptr<im2_channel> im2_channel_builder::accept(uint64_t& peer_id) {
    auto skt = m_socket_builder.accept();
    auto channel = std::make_shared<im2_channel>(skt, false);
    uint64_t id;
    std::string msg;
    channel->recv_text(id, msg);
    peer_id = *((uint64_t*)msg.data());
    boost::endian::big_to_native_inplace(peer_id);
    return channel;
}

// im2_channel_recv_work
void im2_channel_recv_work::do_work() {
    std::shared_ptr<Work> this_obj = shared_from_this();
    WorkUtils::WorkCoCbs co_cbs{[this_obj]() { this_obj->m_wp.wp_yield(0); },
                                [this_obj]() { this_obj->add_self_back_to_main_worker(nullptr); }};
    auto channel = std::make_shared<im2_channel>(
            std::make_shared<WorkUtils::TcpSocket>(
                    m_channel->m_tcp->m_socket_real,
                    co_cbs), true);
    while (true) {
        std::string msg;
        if (!channel->recv_msg(msg)) {
            log_error("channel failed to receive message!");
            break;
        }
        auto light_channel = channel->get_light_channel_from_msg(msg);
        if (light_channel) {
            m_main_worker->add_work(new WorkWrap(
                    std::make_shared<im2_light_channel_server_work>(
                            light_channel, 11111)));
        }
        channel->deal_with_msg(msg);
    }
}

// im2_channel_send_work
void im2_channel_send_work::do_work() {
    std::shared_ptr<Work> this_obj = shared_from_this();
    WorkUtils::WorkCoCbs co_cbs{[this_obj]() { this_obj->m_wp.wp_yield(0); },
                                [this_obj]() { this_obj->add_self_back_to_main_worker(nullptr); }};
    auto channel = std::make_shared<im2_channel>(
            std::make_shared<WorkUtils::TcpSocket>(
                    m_channel->m_tcp->m_socket_real,
                    co_cbs), true);
    auto light_channel = std::make_shared<im2_light_channel>(
            channel,
            channel->generate_light_ch_id(),
            true,
            co_cbs);
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
    std::shared_ptr<Work> this_obj = shared_from_this();
    WorkUtils::WorkCoCbs co_cbs{[this_obj]() { this_obj->m_wp.wp_yield(0); },
                                [this_obj]() { this_obj->add_self_back_to_main_worker(nullptr); }};
    auto light_channel = std::make_shared<im2_light_channel>(
            m_channel->m_main_channel,
            m_channel->m_light_channel_id,
            true,
            co_cbs);
    std::string msg = "msg from: " + std::to_string(m_my_id);
//    WorkUtils::Timer_Asio timer{&(*m_io_worker), shared_from_this()};
    while (true) {
        if (!light_channel->send_text(11111, msg)) {
            log_error("failed to send message to server!");
            break;
        }
        // todo: here can recv_text for this light channel. and not block message in other channels
//        timer.wait_for(5000);
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
