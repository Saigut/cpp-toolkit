#ifndef CPP_TOOLKIT_PROD_IM_SERVER_MOD_JSONRPC_API_HPP
#define CPP_TOOLKIT_PROD_IM_SERVER_MOD_JSONRPC_API_HPP

#include <httplib/httplib.h>
#include "prod_im_dt.h"

class prod_im_server_jsonrpc_api {
public:
    int run();
private:
    int user_register(const prod_im_user_account& user_acco);
    int login(const prod_im_user_account& user_acco);
    int get_contact_list(
            const std::string& user_id,
            std::shared_ptr<prod_im_cont_list>& cont_list);
    int add_contact(const std::string& user_id,
                    const prod_im_contact& cont);
    int del_contact(const std::string& user_id, const std::string& cont_id);
    int client_send_chat_msg(const prod_im_chat_msg& chat_msg);
    int client_get_chat_msg(const std::string& user_id,
                            size_t msg_index,
                            std::shared_ptr<prod_im_chat_msg_list> msg_list);

    int process_req(const httplib::Request& req, httplib::Response& res);

    std::string m_my_id;
};

#endif //CPP_TOOLKIT_PROD_IM_SERVER_MOD_JSONRPC_API_HPP
