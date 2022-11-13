#ifndef CPP_TOOLKIT_APP_PROD_IM_INTERNAL_H
#define CPP_TOOLKIT_APP_PROD_IM_INTERNAL_H

#include <string>
#include <vector>
#include <list>
#include <map>
#include <memory>
#include <system_error>

#include <boost/asio/io_context.hpp>
#include <boost/asio/deadline_timer.hpp>

#include <grpcpp/grpcpp.h>
#include <prod_im_server.grpc.pb.h>

#include <mod_common/log.h>
#include <mod_ring_queue/mod_ring_queue.h>

#include "prod_im_dt.h"
#include "prod_im_helper.h"


int app_prod_im_server(int argc, char** argv);
int app_prod_im_client(int argc, char** argv);


// common

// client
class call_im_server_grpc {
public:
    call_im_server_grpc(std::shared_ptr<grpc::Channel> channel)
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
    void update_list(prod_im_cont_list& contact_list);
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

    int user_register(const std::string& user_pass);
    int login(const std::string& user_pass);
    std::shared_ptr<prod_im_cont_list> get_contact_list();
    int add_contact(const std::string& contact_id,
                    const std::string& contact_name);
    void del_contact(const std::string& contact_id);
    int send_msg_to_contact(const std::string& contact_id,
                            const std::string& chat_msg);
    void process_chat_msg(const std::string& sender_id,
                          const std::string& chat_msg);
    std::shared_ptr<prod_im_chat_msg_list> get_chat_msg();
    void run();
private:
    std::string m_server_ip;
    uint16_t m_server_port;
    uint16_t m_client_port;

    std::string m_my_id;
    size_t m_msg_index = 0;

    prod_im_c_chat_msg_sending m_chat_msg_sending;

    std::shared_ptr<call_im_server_grpc> m_server_grpc_api;
};

int prod_im_client_mod_cli_read_loop();
void prod_im_client_mod_cli_recv_msg(const char* fmt, ...);

static void print_client_cli_usage()
{
    prod_im_client_mod_cli_recv_msg("-| REPL usage eg:\n");
    prod_im_client_mod_cli_recv_msg("-| reg <user passwd>\n");
    prod_im_client_mod_cli_recv_msg("-| login <user passwd>\n");
    prod_im_client_mod_cli_recv_msg("-| cont_list\n");
    prod_im_client_mod_cli_recv_msg("-| cont_add <contact id> <contact name>\n");
    prod_im_client_mod_cli_recv_msg("-| cont_del <contact id>\n");
    prod_im_client_mod_cli_recv_msg("-| msg <contact id> <message>\n");
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
        std::vector<prod_im_chat_msg> user_chat_msg;
    };
public:
    int user_add(const std::string& user_id, const std::string& user_pass);
    void user_del(const std::string& user_id);
    std::shared_ptr<user_info_t> user_find(const std::string& user_id);
    bool user_exist(const std::string& user_id);
    // 0, ok; -1 error; -2 user not exist
    int user_get_chat_msg(const std::string& user_id, prod_im_chat_msg_list& chat_msg);
    // 0, ok; -1 error; -2 user not exist
    int user_get_chat_msg_from(const std::string& user_id,
                               size_t msg_index,
                               prod_im_chat_msg_list& chat_msg);
    // 0, ok; -1 error; -2 user not exist
    int user_add_msg(const std::string& user_id, prod_im_chat_msg& chat_msg);

    int user_contact_add(const std::string& user_id,
                         const std::string& contact_id,
                         const std::string& contact_name);

    void user_contact_del(const std::string& user_id, const std::string& contact_id);

    std::shared_ptr<prod_im_cont_list> user_contact_get_list(const std::string& user_id);
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

enum emIM_S_MAIN_MSG_type {
    emIM_S_MAIN_MSG_type_REG = 0,
    emIM_S_MAIN_MSG_type_LOGIN,
    emIM_S_MAIN_MSG_type_GET_CONT_LIST,
    emIM_S_MAIN_MSG_type_ADD_CONT,
    emIM_S_MAIN_MSG_type_DEL_CONT,
    emIM_S_MAIN_MSG_type_CLIENT_MSG,
    emIM_S_MAIN_MSG_type_GET_CHAT_MSG,
};

struct im_s_main_reg_req {
    prod_im_user_account use_acco;
};
struct im_s_main_reg_res {
    int rst;
};

struct im_s_main_login_req {
    prod_im_user_account use_acco;
    prod_im_s_client_info client_info;
};
struct im_s_main_login_res {
    int rst;
};

struct im_s_main_get_cont_list_req {
    std::string user_id;
};
struct im_s_main_get_cont_list_res {
    int rst;
};

struct im_s_main_add_cont_req {
    std::string user_id;
    prod_im_contact cont;
};
struct im_s_main_add_cont_res {
    int rst;
};

struct im_s_main_del_cont_req {
    std::string user_id;
    std::string cont_id;
};
struct im_s_main_del_cont_res {
    int rst;
};

struct im_s_main_client_msg_req {
    prod_im_chat_msg chat_msg;
};
struct im_s_main_client_msg_res {
    int rst;
};

struct im_s_main_get_chat_msg_req {
    std::string user_id;
    size_t msg_index;
};

using prod_im_s_main_common_cb = std::function<void(std::error_code)>;
using prod_im_s_main_get_cont_list_cb = std::function<void(std::error_code, std::shared_ptr<prod_im_cont_list>)>;
using prod_im_s_main_get_chat_msg_cb = std::function<void(std::error_code, std::shared_ptr<prod_im_chat_msg_list>)>;

class prod_im_s_mod_main_msg {
public:
    explicit prod_im_s_mod_main_msg() {}
    emIM_S_MAIN_MSG_type type;

    im_s_main_reg_req reg;
    im_s_main_login_req login;
    im_s_main_get_cont_list_req get_cont_list;
    im_s_main_add_cont_req add_cont;
    im_s_main_del_cont_req del_cont;
    im_s_main_client_msg_req client_msg;
    im_s_main_get_chat_msg_req get_chat_msg;

    prod_im_s_main_common_cb common_cb;
    prod_im_s_main_get_cont_list_cb get_cont_list_cb;
    prod_im_s_main_get_chat_msg_cb get_chat_msg_cb;
};

class prod_im_s_mod_main_operation {
public:
    explicit prod_im_s_mod_main_operation(boost::asio::io_context& io_ctx)
    : m_user_session(io_ctx),
      m_io_ctx(io_ctx),
      m_op_mpool(4096),
      m_op_queue(4096, m_op_mpool) {
        if (0 != m_op_mpool.init()) {
            log_error("m_op_mpool init failed!!");
            exit(-1);
        }
    }
    ~prod_im_s_mod_main_operation() {
        m_op_mpool.deinit();
    }

    void read_operation();
    int write_operation(prod_im_s_mod_main_msg* msg);

    prod_im_s_mod_main_msg* alloc_msg();
    void free_msg(prod_im_s_mod_main_msg* msg);

private:
    int process_operation(prod_im_s_mod_main_msg* msg);

    int user_register(const prod_im_user_account& user_acco);
    int login(const prod_im_user_account& user_acco,
              const prod_im_s_client_info& client_info);
    std::shared_ptr<prod_im_cont_list> get_contact_list(const std::string& user_id);
    int add_contact(const std::string& user_id,
                    const prod_im_contact& cont);
    int del_contact(const std::string& user_id, const std::string& contact_id);
    int client_send_chat_msg(prod_im_chat_msg& chat_msg);
    std::shared_ptr<prod_im_chat_msg_list> co_func_get_chat_msg(const std::string& user_id, size_t msg_index);


    prod_im_s_mod_uinfo m_user_info;
    prod_im_s_mod_user_session m_user_session;
    boost::asio::io_context& m_io_ctx;

    std::mutex op_writer_lock;
    std::atomic<bool> m_op_msg_notification_on = false;
    std::function<void()> notify_to_read_op_func = nullptr;
    im_s_op_msg_mpool m_op_mpool;
    ring_queue m_op_queue;

    std::map<std::string, std::list<std::function<void()>>> get_chat_msg_notify_func;
};

class prod_im_s_mod_main {
public:
    explicit prod_im_s_mod_main(boost::asio::io_context& io_ctx)
    : m_operation(io_ctx) {}
    /// Fixme:   deal with when api thread is broken
    int user_register(const std::string& user_id,
                      const std::string& user_pass);
    int login(const std::string& user_id, const std::string& user_pass, const std::string& client_ip,
              uint16_t client_port);
    std::shared_ptr<prod_im_cont_list>
    get_contact_list(const std::string& user_id);
    int add_contact(const std::string& user_id,
                    const std::string& contact_id,
                    const std::string& contact_name);
    int del_contact(const std::string& user_id,
                    const std::string& contact_id);
    void client_send_chat_msg(const std::string& sender_id,
                              const std::string& receiver_id,
                              const std::string& chat_msg);
    std::shared_ptr<prod_im_chat_msg_list> client_get_chat_msg(const std::string& user_id, size_t msg_index);

    int user_register(const prod_im_user_account& user_acco,
                      prod_im_s_main_common_cb&& cb);
    int login(const prod_im_user_account& user_acco,
              const prod_im_s_client_info& client_info,
              prod_im_s_main_common_cb&& cb);
    int get_contact_list(const std::string& user_id,
                         prod_im_s_main_get_cont_list_cb&& cb);
    int add_contact(const std::string& user_id,
                    const prod_im_contact& cont,
                    prod_im_s_main_common_cb&& cb);
    int del_contact(const std::string& user_id, const std::string& contact_id, prod_im_s_main_common_cb&& cb);
    int client_send_chat_msg(prod_im_chat_msg& chat_msg, prod_im_s_main_common_cb&& cb);
    int client_get_chat_msg(const std::string& user_id, size_t msg_index, prod_im_s_main_get_chat_msg_cb&& cb);

    void run();

private:

    prod_im_s_mod_main_operation m_operation;
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

    ::grpc::Status client_send_chat_msg(::grpc::ServerContext* context,
                                        const ::prod_im_server::send_chat_msg_req* request,
                                        ::prod_im_server::send_chat_msg_res* response) override;

    ::grpc::Status client_get_chat_msg(::grpc::ServerContext* context,
                                       const prod_im_server::get_chat_msg_req* request,
                                       prod_im_server::get_chat_msg_res* response) override;
};


#endif //CPP_TOOLKIT_APP_PROD_IM_INTERNAL_H
