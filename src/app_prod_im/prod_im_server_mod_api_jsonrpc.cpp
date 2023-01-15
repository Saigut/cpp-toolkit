#include "prod_im_server_mod_api_jsonrpc.hpp"

#include <mod_common/log.hpp>
#include <mod_common/expect.hpp>

#include "prod_im_server_mod_main.hpp"


using json = nlohmann::json;

extern std::shared_ptr<prod_im_s_mod_main> g_server_main;

int prod_im_server_api_jsonrpc::user_register(nlohmann::json& params_obj,
                                              nlohmann::json& result_obj)
{
    auto user_id = params_obj["user_id"].get<std::string>();
    auto user_pass = params_obj["user_pass"].get<std::string>();

    int ret;
    ret = g_server_main->user_register(user_id, user_pass);
    expect_ret_val(0 == ret, ret);

    return 0;
}

int prod_im_server_api_jsonrpc::login(nlohmann::json& params_obj,
                                      nlohmann::json& result_obj)
{
    auto user_id = params_obj["user_id"].get<std::string>();
    auto user_pass = params_obj["user_pass"].get<std::string>();

    int ret;
    ret = g_server_main->login(user_id, user_pass, "", 0);
    expect_ret_val(0 == ret, ret);

    return 0;
}

int
prod_im_server_api_jsonrpc::get_contact_list(nlohmann::json& params_obj,
                                             nlohmann::json& result_obj)
{
    auto user_id = params_obj["user_id"].get<std::string>();

    auto rst = g_server_main->get_contact_list(user_id);

    json contact_list_obj;
    for (auto& cont : *rst) {
        json cont_obj;
        cont_obj["contact_id"] = cont.contact_id;
        cont_obj["contact_name"] = cont.contact_name;
        contact_list_obj.emplace_back(std::move(cont_obj));
    }
    result_obj["contact_list"] = contact_list_obj;

    return 0;
}

int prod_im_server_api_jsonrpc::add_contact(nlohmann::json& params_obj,
                                            nlohmann::json& result_obj)
{
    auto user_id = params_obj["user_id"].get<std::string>();
    auto contact_id = params_obj["contact_id"].get<std::string>();
    auto contact_name = params_obj["contact_name"].get<std::string>();

    int ret;
    ret = g_server_main->add_contact(user_id, contact_id, contact_name);
    expect_ret_val(0 == ret, ret);

    return 0;
}

int prod_im_server_api_jsonrpc::del_contact(nlohmann::json& params_obj,
                                            nlohmann::json& result_obj)
{
    auto user_id = params_obj["user_id"].get<std::string>();
    auto contact_id = params_obj["contact_id"].get<std::string>();

    int ret;
    ret = g_server_main->del_contact(user_id, contact_id);
    expect_ret_val(0 == ret, ret);

    return -1;
}

int
prod_im_server_api_jsonrpc::client_send_chat_msg(nlohmann::json& params_obj,
                                                 nlohmann::json& result_obj)
{
    auto sender_id = params_obj["sender_id"].get<std::string>();
    auto receiver_id = params_obj["receiver_id"].get<std::string>();
    auto chat_msg = params_obj["chat_msg"].get<std::string>();

    prod_im_chat_msg st_chat_msg{sender_id, receiver_id, chat_msg};
    g_server_main->client_send_chat_msg(st_chat_msg);

    return 0;
}

int prod_im_server_api_jsonrpc::client_get_chat_msg(nlohmann::json& params_obj,
                                                    nlohmann::json& result_obj)
{
    auto user_id = params_obj["user_id"].get<std::string>();
    auto msg_index = params_obj["msg_index"].get<size_t>();

    auto rst = g_server_main->client_get_chat_msg(user_id, msg_index);

    json msg_list_obj;
    for (auto& msg : *rst) {
        json msg_obj;
        msg_obj["sender_id"] = msg.sender_id;
        msg_obj["receiver_id"] = msg.receiver_id;
        msg_obj["chat_msg"] = msg.chat_msg;
        msg_list_obj.emplace_back(std::move(msg_obj));
    }
    result_obj["msg_list"] = msg_list_obj;

    return 0;
}

int prod_im_server_api_jsonrpc::process_req_method(std::string& method,
                                                   nlohmann::json& params_obj,
                                                   nlohmann::json& result_obj)
{
    if ("register" == method) {
        return user_register(params_obj, result_obj);

    } else if ("login" == method) {
        return login(params_obj, result_obj);

    } else if ("get_contact_list" == method) {
        return get_contact_list(params_obj, result_obj);

    } else if ("add_contact" == method) {
        return add_contact(params_obj, result_obj);

    } else if ("del_contact" == method) {
        return del_contact(params_obj, result_obj);

    } else if ("client_send_chat_msg" == method) {
        return client_send_chat_msg(params_obj, result_obj);

    } else if ("client_get_chat_msg" == method) {
        return client_get_chat_msg(params_obj, result_obj);

    } else {
        log_error("invalid method: %s", method.c_str());
        return -1;
    }
}

int prod_im_server_api_jsonrpc::process_req(const httplib::Request& req,
                                            httplib::Response& res)
{
    auto& json_str = req.body;

    json error_res_obj = {
        {"jsonrpc", "2.0"},
        {"error",
            {
                {"code", -1},
                {"message", "Invalid json"}
            }
        }
    };

    json json_obj;
    try {
        json_obj = json::parse(json_str);
    } catch (...) {
        log_error("json parse failed! json_str: %s", json_str.c_str());
        res.set_content(error_res_obj.dump(), "text/json");
        return -1;
    }

    try {
        // root.method
        auto method_obj = json_obj["method"];
        auto method = method_obj.get<std::string>();
        // root.params
        auto params_obj = json_obj["params"];
        json result_obj;

        json res_obj = {
                {"jsonrpc", "2.0"}
        };

        int ret;
        ret = process_req_method(method, params_obj, result_obj);
        if (0 == ret) {
            res_obj["result"] = result_obj;
        } else {
            json error_obj = {
                    {"code", ret},
                    {"message", ""},
            };
            res_obj["error"] = error_obj;
        }

        res.set_content(res_obj.dump(), "text/json");

    } catch (...) {
        log_error("invalid parameter!");
        error_res_obj["error"]["message"] = "Invalid parameter";
        res.set_content(error_res_obj.dump(), "text/json");
        return -1;
    }

    return 0;
}

int prod_im_server_api_jsonrpc::run()
{
    // HTTPS
    // openssl req -newkey rsa:2048 -new -nodes -x509 -days 3650 -keyout key.pem -out cert.pem
//    httplib::SSLServer svr("./cert.pem", "./key.pem");
    httplib::Server svr;

    svr.Put("/jsonrpc", [this](const httplib::Request& req, httplib::Response& res) {
        log_info("req method: %s", req.method.c_str());
        log_info("req content length: %zu", req.content_length_);
        log_info("req body: %s", req.body.c_str());
        this->process_req(req, res);
    });

    log_info("jsonrpc listen address: 0.0.0.0:60080");
    svr.listen("0.0.0.0", 60080);
    return 0;
}
