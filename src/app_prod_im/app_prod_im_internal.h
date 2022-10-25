#ifndef CPP_TOOLKIT_APP_PROD_IM_INTERNAL_H
#define CPP_TOOLKIT_APP_PROD_IM_INTERNAL_H

#include <string>
#include <vector>
#include <map>
#include <memory>

#include <boost/asio/io_context.hpp>
#include <boost/asio/deadline_timer.hpp>

#include <prod_im_server.grpc.pb.h>


int app_prod_im_server(int argc, char** argv);
int app_prod_im_client(int argc, char** argv);


// common
struct prod_im_contact {
    std::string contact_id;
    std::string contact_name;
};


// client
class prod_im_c_mod_main {
public:
    int user_register(std::string& user_id,
                      std::string& user_pass);
    int login(std::string& user_id,
              std::string& user_pass);
    std::vector<prod_im_contact>&& get_contact_list();
    int add_contact(std::string& contact_id,
                    std::string& contact_name);
    void del_contact(std::string& contact_id);
    int send_msg_to_contact(std::string& contact_id,
                            std::string& chat_msg);
    void recv_chat_msg(std::string& sender_id,
                       std::string& chat_msg);
    void run();
};

class prod_im_c_mod_login_session {
public:
    int open(int io_skt);
    void close();
    int get_io_port();
};

class prod_im_c_mod_contacts {
public:
    void update_list(std::vector<prod_im_contact>& contact_list);
    int add_contact(std::string& contact_id, std::string& contact_name);
    int del_contact();
};

class prod_im_c_chat_msg_sending {
public:
    int send_msg(std::string& receiver_id, std::string& msg_content);
};

class prod_im_c_chat_msg_receiving {
public:
    void run();
};


// server
struct prod_im_s_user_session {
    prod_im_s_user_session(const std::string& _user_id,
                           const std::string& _client_ip,
                           std::shared_ptr<boost::asio::deadline_timer> _timer)
                           : user_id(_user_id),
                             client_ip(_client_ip),
                             timer(_timer) {}
    std::string user_id;
    std::string client_ip;
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

    std::vector <prod_im_contact>&& user_contact_get_list(const std::string& user_id);
private:
    std::map<std::string, user_info_t> m_users;
};

class prod_im_s_mod_user_session {
public:
    explicit prod_im_s_mod_user_session(boost::asio::io_context& io_ctx)
    : m_io_ctx(io_ctx) {}
    int add(const std::string& user_id, const std::string& client_ip);
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
    int relay_msg(const std::string& peer_ip, const std::string& sender_id,
                  const std::string& receiver_id, const std::string& chat_content);
};

class prod_im_s_mod_main {
public:
    explicit prod_im_s_mod_main() : m_user_session(m_io_ctx) {}
    int user_register(const std::string& user_id,
                      const std::string& user_pass);
    int login(const std::string& user_id, const std::string& user_pass, const std::string& client_ip);
    std::vector<prod_im_contact>&&
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
    boost::asio::io_context m_io_ctx;
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
