#ifndef CPP_TOOLKIT_PROD_IM_SERVER_MOD_API_GRPC_HPP
#define CPP_TOOLKIT_PROD_IM_SERVER_MOD_API_GRPC_HPP

#include <grpcpp/grpcpp.h>
#include <prod_im_server.grpc.pb.h>

class prod_im_server_api_grpc final : public prod_im_server::prod_im_server_service::Service {
public:
    ::grpc::Status user_register(::grpc::ServerContext* context,
                                 const ::prod_im_server::user_register_req* request,
                                 ::prod_im_server::user_register_res* response) override;

    ::grpc::Status login(::grpc::ServerContext* context,
                         const ::prod_im_server::login_req* request,
                         ::prod_im_server::login_res* response) override;

    ::grpc::Status
    get_contact_list(::grpc::ServerContext* context,
                     const ::prod_im_server::get_contact_list_req* request,
                     ::prod_im_server::get_contact_list_res* response) override;

    ::grpc::Status add_contact(::grpc::ServerContext* context,
                               const ::prod_im_server::add_contact_req* request,
                               ::prod_im_server::add_contact_res* response) override;

    ::grpc::Status del_contact(::grpc::ServerContext* context,
                               const ::prod_im_server::del_contact_req* request,
                               ::prod_im_server::del_contact_res* response) override;

    ::grpc::Status client_send_chat_msg(::grpc::ServerContext* context,
                                        const ::prod_im_server::send_chat_msg_req* request,
                                        ::prod_im_server::send_chat_msg_res* response) override;

    ::grpc::Status client_get_chat_msg(::grpc::ServerContext* context,
                                       const prod_im_server::get_chat_msg_req* request,
                                       prod_im_server::get_chat_msg_res* response) override;

    int run();
};


#endif //CPP_TOOLKIT_PROD_IM_SERVER_MOD_API_GRPC_HPP
