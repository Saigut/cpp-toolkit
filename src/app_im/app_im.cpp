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

// im2_channel_builder
std::shared_ptr<im2_channel> im2_channel_builder::connect(std::shared_ptr<Work> consignor_work,
                                                                  uint64_t my_id,
                                                                  const std::string& addr_str,
                                                                  uint16_t port) {
    auto connector_asio = std::make_shared<::WorkUtils::TcpSocketConnector_Asio>(&(*m_io_worker));
    expect_ret_val(0 == connector_asio->connect(consignor_work, addr_str, port), nullptr);
    auto channel = std::make_shared<im2_channel>(connector_asio, m_main_worker);
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
    auto channel = std::make_shared<im2_channel>(socket_asio, m_main_worker);
    uint64_t id;
    std::string msg;
    channel->recv_text(consignor_work, id, msg);
    peer_id = *((uint64_t*)msg.data());
    boost::endian::big_to_native_inplace(peer_id);
    return channel;
}

int app_im(int argc, char** argv)
{
    if (argc < 2) {
        log_error("wrong parameters.");
//        print_usage();
        return -1;
    }

    if (0 == strcmp(argv[1], "c")) {
        return app_im_client_new(argc - 1, argv + 1);
    } else if (0 == strcmp(argv[1], "s")) {
        return app_im_server_new(argc - 1, argv + 1);
    } else {
        log_error("wrong parameters.");
//        print_usage();
        return -1;
    }
}
