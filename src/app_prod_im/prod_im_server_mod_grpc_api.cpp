#include "app_prod_im_internal.h"

#include <grpcpp/grpcpp.h>
#include <prod_im_server.grpc.pb.h>
#include <mod_common/log.h>

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
using prod_im_server::prod_im_server_service;

extern std::shared_ptr<prod_im_s_mod_main> g_server_main;


::grpc::Status prod_im_server_grpc_api_impl::user_register(::grpc::ServerContext* context,
                                                           const ::prod_im_server::user_register_req* request,
                                                           ::prod_im_server::user_register_res* response)
{
    response->set_result(
            g_server_main->user_register(request->user_id(), request->user_pass()));
    return Status::OK;
}

::grpc::Status prod_im_server_grpc_api_impl::login(::grpc::ServerContext* context,
                                                   const ::prod_im_server::login_req* request,
                                                   ::prod_im_server::login_res* response) {
    const auto& peer_addr = context->peer();
    std::string peer_ip_addr;
    std::size_t maohao_pos = peer_addr.rfind(':');
    if (maohao_pos == 0) {
        peer_ip_addr = peer_addr;
    } else {
        peer_ip_addr = peer_addr.substr(0, maohao_pos);
    }
    log_info("Peer: %s, peer ip address: %s", peer_addr.c_str(), peer_ip_addr.c_str());
    response->set_result(
            g_server_main->login(request->user_id(), request->user_pass(),
                                peer_ip_addr, request->client_port()));
    return Status::OK;
}

::grpc::Status prod_im_server_grpc_api_impl::get_contact_list(::grpc::ServerContext* context,
                                                              const ::prod_im_server::get_contact_list_req* request,
                                                              ::prod_im_server::get_contact_list_res* response) {
    auto rst = g_server_main->get_contact_list(request->user_id());
    if (rst) {
        for (auto& contact : *rst) {
            auto a_contact = response->add_contact_list();
            a_contact->set_contact_id(contact.contact_id);
            a_contact->set_contact_name(contact.contact_name);
        }
    }
    response->set_result(0);
    return Status::OK;
}

::grpc::Status prod_im_server_grpc_api_impl::add_contact(::grpc::ServerContext* context,
                                                         const ::prod_im_server::add_contact_req* request,
                                                         ::prod_im_server::add_contact_res* response) {
    response->set_result(
            g_server_main->add_contact(request->user_id(), request->contact_id(), request->contact_name()));
    return Status::OK;
}

::grpc::Status prod_im_server_grpc_api_impl::del_contact(::grpc::ServerContext* context,
                                                         const ::prod_im_server::del_contact_req* request,
                                                         ::prod_im_server::del_contact_res* response)
{
    response->set_result(g_server_main->del_contact(request->user_id(), request->contact_id()));
    return Status::OK;
}

::grpc::Status prod_im_server_grpc_api_impl::send_chat_msg(::grpc::ServerContext* context,
                                                           const ::prod_im_server::send_chat_msg_req* request,
                                                           ::prod_im_server::send_chat_msg_res* response) {
    g_server_main->client_chat_msg(request->sender_id(),
                                   request->receiver_id(),
                                   request->chat_msg());
    response->set_result(0);
    return Status::OK;
}

::grpc::Status prod_im_server_grpc_api_impl::get_msg(::grpc::ServerContext* context,
                                                     const ::prod_im_server::get_msg_req* request,
                                                     ::prod_im_server::get_msg_res* response)
{
    auto msg_list = g_server_main->get_chat_msg(request->user_id(),
                                                request->msg_index());
    if (!msg_list || msg_list->empty()) {
        response->set_result(-1);
    } else {
        for (auto& msg : *msg_list) {
            auto pb_msg = response->add_msg_list();
            pb_msg->set_sender_id(msg.sender_id);
            pb_msg->set_receiver_id(msg.receiver_id);
            pb_msg->set_chat_msg(msg.chat_msg);
        }
        response->set_result(0);
    }
    return Status::OK;
}
