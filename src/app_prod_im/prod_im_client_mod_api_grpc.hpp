#ifndef CPP_TOOLKIT_PROD_IM_CLIENT_MOD_API_GRPC_HPP
#define CPP_TOOLKIT_PROD_IM_CLIENT_MOD_API_GRPC_HPP

#include <memory>
#include <string>
#include <grpcpp/channel.h>
#include <prod_im_server.grpc.pb.h>
#include "prod_im_dt.hpp"

class call_im_server_api_grpc {
public:
    call_im_server_api_grpc(std::shared_ptr<grpc::Channel> channel)
            /* Caution: Must include grpcpp/channel for grpc::Channel!*/
            : m_stub(prod_im_server::prod_im_server_service::NewStub(channel)) {}

    int user_register(const std::string& user_id,
                      const std::string& user_pass);

    int login(const std::string& user_id, const std::string& user_pass, uint16_t client_port);

    std::shared_ptr<prod_im_cont_list> get_contact_list(
            const std::string& user_id);

    int add_contact(const std::string& user_id,
                    const std::string& contact_id,
                    const std::string& contact_name);

    int del_contact(const std::string& user_id,
                    const std::string& contact_id);

    int client_send_chat_msg(const std::string& sender_id,
                             const std::string& receiver_id,
                             const std::string& chat_msg);

    std::shared_ptr<prod_im_chat_msg_list> client_get_chat_msg(const std::string& user_id, size_t msg_index);

private:
    std::unique_ptr<prod_im_server::prod_im_server_service::Stub> m_stub;
};


#endif //CPP_TOOLKIT_PROD_IM_CLIENT_MOD_API_GRPC_HPP
