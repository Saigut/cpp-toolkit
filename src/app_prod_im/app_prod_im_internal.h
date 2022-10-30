#ifndef CPP_TOOLKIT_APP_PROD_IM_INTERNAL_H
#define CPP_TOOLKIT_APP_PROD_IM_INTERNAL_H

#include <string>
#include <vector>
#include <map>
#include <memory>

#include <boost/asio/io_context.hpp>
#include <boost/asio/deadline_timer.hpp>

#include <grpcpp/grpcpp.h>
#include <prod_im_client.grpc.pb.h>
#include <prod_im_server.grpc.pb.h>


int app_prod_im_server(int argc, char** argv);
int app_prod_im_client(int argc, char** argv);


// common
struct prod_im_contact {
    std::string contact_id;
    std::string contact_name;
};


// client
class call_im_server_grpc {
public:
    call_im_server_grpc(std::shared_ptr<grpc::Channel> channel)
            : m_stub(prod_im_server::prod_im_server_service::NewStub(channel)) {}

    int user_register(const std::string& user_id,
                      const std::string& user_pass);

    int login(const std::string& user_id, const std::string& user_pass, uint16_t client_port);

    std::shared_ptr<std::vector<prod_im_contact>> get_contact_list(
            const std::string& user_id);

    int add_contact(const std::string& user_id,
                    const std::string& contact_id,
                    const std::string& contact_name);

    int del_contact(const std::string& user_id,
                    const std::string& contact_id);

    int send_chat_msg(const std::string& sender_id,
                      const std::string& receiver_id,
                      const std::string& chat_msg);

private:
    std::unique_ptr<prod_im_server::prod_im_server_service::Stub> m_stub;
};

class prod_im_c_mod_login_session {
public:
    prod_im_c_mod_login_session(const std::string& user_id,
                                const std::string& user_pass,
                                std::shared_ptr<call_im_server_grpc> server_grpc_api)
                                : m_user_id(user_id),
                                  m_user_pass(user_pass),
                                  m_server_grpc_api(server_grpc_api)
                                  {}
    int open();
    void close();
private:
    std::string m_user_id;
    std::string m_user_pass;
    std::shared_ptr<call_im_server_grpc> m_server_grpc_api;
};

class prod_im_c_mod_contacts {
public:
    void update_list(std::vector<prod_im_contact>& contact_list);
    int add_contact(std::string& contact_id, std::string& contact_name);
    int del_contact(std::string& contact_id);
private:
    std::map<std::string, prod_im_contact> m_contacts;
};

class prod_im_c_chat_msg_sending {
public:
    int send_msg(const std::string& server_ip, uint16_t server_port, const std::string& sender_id,
                 const std::string& receiver_id, const std::string& msg_content);
};

class prod_im_c_chat_msg_receiving {
public:
    void run();
};

class prod_im_c_mod_main {
public:
//    explicit prod_im_c_mod_main(
//            std::string m_server_ip;
//    uint16_t m_server_port;
//    uint16_t m_client_port;
//
//    std::string m_my_id;
//            std::shared_ptr<call_im_server_grpc> server_grpc_api)
//            : m_server_grpc_api(server_grpc_api) {}
    prod_im_c_mod_main(const std::string& mServerIp,
                       uint16_t mServerPort,
                       uint16_t mClientPort,
                       const std::string& mMyId,
                       std::shared_ptr<call_im_server_grpc> mServerGrpcApi)
            : m_server_ip(mServerIp),
              m_server_port(mServerPort),
              m_client_port(mClientPort),
              m_my_id(mMyId),
              m_server_grpc_api(mServerGrpcApi) {}

    int user_register(const std::string& user_id,
                      const std::string& user_pass);
    int login(const std::string& user_id,
              const std::string& user_pass);
    std::shared_ptr<std::vector<prod_im_contact>> get_contact_list();
    int add_contact(const std::string& contact_id,
                    const std::string& contact_name);
    void del_contact(const std::string& contact_id);
    int send_msg_to_contact(const std::string& contact_id,
                            const std::string& chat_msg);
    void recv_chat_msg(const std::string& sender_id,
                       const std::string& chat_msg);
    void run();
private:
    std::string m_server_ip;
    uint16_t m_server_port;
    uint16_t m_client_port;

    std::string m_my_id;

    prod_im_c_chat_msg_sending m_chat_msg_sending;

    std::shared_ptr<call_im_server_grpc> m_server_grpc_api;
};

class prod_im_client_grpc_api_impl final : public prod_im_client::prod_im_client_service::Service {
public:
    ::grpc::Status send_chat_msg(::grpc::ServerContext* context, const ::prod_im_client::send_chat_msg_req* request,
                                 ::prod_im_client::send_chat_msg_res* response) override;
};

int prod_im_client_mod_cli(int argc, char** argv);

static void print_client_cli_usage()
{
    printf("Usage eg:\n");
    printf("./<program> <user id> reg <user id> <user passwd>\n");
    printf("./<program> <user id> login <user id> <user passwd>\n");
    printf("./<program> <user id> cont_list <user id>\n");
    printf("./<program> <user id> cont_add <contact id> <contact name>\n");
    printf("./<program> <user id> cont_del <contact id>\n");
    printf("./<program> <user id> msg <contact id> <message>\n");
}

// server
struct prod_im_s_user_session {
    prod_im_s_user_session(const std::string& _user_id,
                           const std::string& _client_ip,
                           uint16_t _client_port,
                           std::shared_ptr<boost::asio::deadline_timer> _timer)
                           : user_id(_user_id),
                             client_ip(_client_ip),
                             client_port(_client_port),
                             timer(_timer) {}
    std::string user_id;
    std::string client_ip;
    uint16_t client_port;
    std::shared_ptr<boost::asio::deadline_timer> timer;
};

class prod_im_s_mod_uinfo {
private:
    struct user_info_t {
        std::string user_id;
        std::string user_pass;
        std::map<std::string, prod_im_contact> user_contacts;
    };
public:
    int user_add(const std::string& user_id, const std::string& user_pass);
    void user_del(const std::string& user_id);
    std::shared_ptr<user_info_t> user_find(const std::string& user_id);

    int user_contact_add(const std::string& user_id,
                         const std::string& contact_id,
                         const std::string& contact_name);

    void user_contact_del(const std::string& user_id, const std::string& contact_id);

    std::shared_ptr<std::vector<prod_im_contact>> user_contact_get_list(const std::string& user_id);
private:
    std::map<std::string, user_info_t> m_users;
};

class prod_im_s_mod_user_session {
public:
    explicit prod_im_s_mod_user_session(boost::asio::io_context& io_ctx)
    : m_io_ctx(io_ctx) {}
    int add(const std::string& user_id, const std::string& client_ip, uint16_t client_port);
    void del(const std::string& user_id);
    std::shared_ptr<prod_im_s_user_session> find(const std::string& user_id);
private:
    std::map<std::string, prod_im_s_user_session> m_user_sessions;
    boost::asio::io_context& m_io_ctx;
};

class prod_im_s_mod_chat_msg_receiving {
public:
    void run();
};

class prod_im_s_mod_chat_msg_relay {
public:
    int relay_msg(const std::string& peer_ip, uint16_t peer_port, const std::string& sender_id,
                  const std::string& receiver_id, const std::string& chat_content);
};

class prod_im_s_mod_main {
public:
    explicit prod_im_s_mod_main(boost::asio::io_context& io_ctx)
    : m_user_session(io_ctx), m_io_ctx(io_ctx) {}
    int user_register(const std::string& user_id,
                      const std::string& user_pass);
    int login(const std::string& user_id, const std::string& user_pass, const std::string& client_ip,
              uint16_t client_port);
    std::shared_ptr<std::vector<prod_im_contact>>
    get_contact_list(const std::string& user_id);
    int add_contact(const std::string& user_id,
                    const std::string& contact_id,
                    const std::string& contact_name);
    int del_contact(const std::string& user_id,
                    const std::string& contact_id);
    void recv_chat_msg(const std::string& sender_id,
                       const std::string& receiver_id,
                       const std::string& chat_msg);
    void run();
private:
    prod_im_s_mod_uinfo m_user_info;
    prod_im_s_mod_user_session m_user_session;
    prod_im_s_mod_chat_msg_relay m_chat_msg_relay;
    boost::asio::io_context& m_io_ctx;
};

class prod_im_server_grpc_api_impl final : public prod_im_server::prod_im_server_service::Service {
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

    ::grpc::Status send_chat_msg(::grpc::ServerContext* context,
                                 const ::prod_im_server::send_chat_msg_req* request,
                                 ::prod_im_server::send_chat_msg_res* response) override;
};


#endif //CPP_TOOLKIT_APP_PROD_IM_INTERNAL_H
