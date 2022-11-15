#include "prod_im_server_mod_jsonrpc_api.hpp"

#include <nlohmann/json.hpp>
#include <mod_common/log.h>
#include <mod_common/expect.h>


int prod_im_server_jsonrpc_api::process_req(const httplib::Request& req,
                                            httplib::Response& res)
{
    using json = nlohmann::json;
    auto& json_str = req.body;

    json json_obj;
    try {
        json_obj = json::parse(json_str);
    } catch (...) {
        log_error("json parse failed!");
        return -1;
    }

    try {
        // root.method
        auto method_obj = json_obj["method"];
        // root.params
        auto params_obj = json_obj["params"];

        auto method = method_obj.get<std::string>();

        if ("register" == method) {
            auto user_id_obj = params_obj["user_id"];
            auto user_pass_obj = params_obj["user_pass"];
            auto user_id = user_id_obj.get<std::string>();
            auto user_pass = user_pass_obj.get<std::string>();

            prod_im_user_account user_acco{ user_id, user_pass };
            expect_ret_val(0 == user_register(user_acco), -1);

            json res_obj = {
                    {"jsonrpc", "2.0"},
                    {"result", 0}
            };
            res.set_content(res_obj.dump(), "text/json");

            return 0;

        } else if ("login" == method) {
            auto user_id_obj = params_obj["user_id"];
            auto user_pass_obj = params_obj["user_pass"];
            auto user_id = user_id_obj.get<std::string>();
            auto user_pass = user_pass_obj.get<std::string>();

            prod_im_user_account user_acco{ user_id, user_pass };
            expect_ret_val(0 == login(user_acco), -1);

            json res_obj = {
                    {"jsonrpc", "2.0"},
                    {"result", 0}
            };
            res.set_content(res_obj.dump(), "text/json");

            return 0;

        } else if ("get_contact_list" == method) {
            std::shared_ptr<prod_im_cont_list> cont_list;
            expect_ret_val(0 == get_contact_list(m_my_id, cont_list), -1);

            json res_obj = {
                    {"jsonrpc", "2.0"},
                    {"result", 0}
            };
            res.set_content(res_obj.dump(), "text/json");

            return 0;

        } else if ("add_contact" == method) {
            auto contact_id_obj = params_obj["contact_id"];
            auto contact_name_obj = params_obj["contact_name"];
            auto contact_id = contact_id_obj.get<std::string>();
            auto contact_name = contact_name_obj.get<std::string>();

            prod_im_contact cont{ contact_id, contact_name };
            expect_ret_val(0 == add_contact(m_my_id, cont), -1);

            json res_obj = {
                    {"jsonrpc", "2.0"},
                    {"result", 0}
            };
            res.set_content(res_obj.dump(), "text/json");

            return 0;
        } else if ("del_contact" == method) {

        } else if ("client_send_chat_msg" == method) {

        } else if ("client_get_chat_msg" == method) {

        } else {
            log_error("invalid method: %s", method.c_str());
            return -1;
        }
    } catch (...) {
        log_error("invalid jsonrpc msg!");
        return -1;
    }

    return 0;
}

int prod_im_server_jsonrpc_api::user_register(const prod_im_user_account& user_acco)
{
    return -1;
}

int prod_im_server_jsonrpc_api::login(const prod_im_user_account& user_acco)
{
    return -1;
}

int
prod_im_server_jsonrpc_api::get_contact_list(const std::string& user_id, std::shared_ptr<prod_im_cont_list>& cont_list)
{
    return -1;
}

int prod_im_server_jsonrpc_api::add_contact(const std::string& user_id,
                                            const prod_im_contact& cont)
{
    return -1;
}

int prod_im_server_jsonrpc_api::del_contact(const std::string& user_id, const std::string& cont_id)
{
    return -1;
}

int prod_im_server_jsonrpc_api::client_send_chat_msg(const prod_im_chat_msg& chat_msg)
{
    return -1;
}

int prod_im_server_jsonrpc_api::client_get_chat_msg(const std::string& user_id, size_t msg_index,
                                                    std::shared_ptr<prod_im_chat_msg_list> msg_list)
{
    return -1;
}
