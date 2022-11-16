#ifndef CPP_TOOLKIT_PROD_IM_SERVER_MOD_JSONRPC_API_HPP
#define CPP_TOOLKIT_PROD_IM_SERVER_MOD_JSONRPC_API_HPP

#include <httplib/httplib.h>
#include <nlohmann/json.hpp>
#include "prod_im_dt.h"

class prod_im_server_jsonrpc_api {
public:
    int run();
private:
    int user_register(nlohmann::json& params_obj,
                      nlohmann::json& result_obj);
    int login(nlohmann::json& params_obj,
              nlohmann::json& result_obj);
    int get_contact_list(nlohmann::json& params_obj,
                         nlohmann::json& result_obj);
    int add_contact(nlohmann::json& params_obj,
                    nlohmann::json& result_obj);
    int del_contact(nlohmann::json& params_obj,
                    nlohmann::json& result_obj);
    int client_send_chat_msg(nlohmann::json& params_obj,
                             nlohmann::json& result_obj);
    int client_get_chat_msg(nlohmann::json& params_obj,
                            nlohmann::json& result_obj);

    int process_req_method(std::string& method,
                           nlohmann::json& params_obj,
                           nlohmann::json& result_obj);
    int process_req(const httplib::Request& req, httplib::Response& res);
};

#endif //CPP_TOOLKIT_PROD_IM_SERVER_MOD_JSONRPC_API_HPP
