#ifndef CPP_TOOLKIT_PROD_IM_SERVER_MOD_MAIN_HPP
#define CPP_TOOLKIT_PROD_IM_SERVER_MOD_MAIN_HPP

#include <string>
#include <memory>
#include <system_error>
#include <functional>
#include <boost/asio/io_context.hpp>
#include <mod_common/log.hpp>
#include <mod_ring_queue/mod_ring_queue.hpp>
#include <list>
#include "prod_im_dt.hpp"
#include "prod_im_server_mod_user_info.hpp"
#include "prod_im_server_mod_user_session.hpp"
#include "prod_im_helper.hpp"

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
    std::function<void(int)> notify_to_read_op_func = nullptr;
    im_s_op_msg_mpool m_op_mpool;
    ring_queue m_op_queue;

    std::map<std::string, std::list<std::function<void(int)>>> get_chat_msg_notify_func;
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
    void client_send_chat_msg(prod_im_chat_msg& chat_msg);
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

#endif //CPP_TOOLKIT_PROD_IM_SERVER_MOD_MAIN_HPP
