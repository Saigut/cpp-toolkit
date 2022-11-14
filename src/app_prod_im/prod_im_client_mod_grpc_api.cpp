#include "app_prod_im_internal.h"

#include <prod_im_server.grpc.pb.h>
#include <mod_common/log.h>

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;

using prod_im_server::user_register_req;
using prod_im_server::user_register_res;
using prod_im_server::login_req;
using prod_im_server::login_res;
using prod_im_server::get_contact_list_req;
using prod_im_server::get_contact_list_res;
using prod_im_server::add_contact_req;
using prod_im_server::add_contact_res;
using prod_im_server::del_contact_req;
using prod_im_server::del_contact_res;
using prod_im_server::send_chat_msg_req;
using prod_im_server::send_chat_msg_res;
using prod_im_server::get_chat_msg_req;
using prod_im_server::get_chat_msg_res;


int call_im_server_grpc::user_register(const std::string& user_id,
                                       const std::string& user_pass) {
    user_register_req req;
    req.set_user_id(user_id);
    req.set_user_pass(user_pass);

    user_register_res res;

    ClientContext context;
    Status status = m_stub->user_register(&context, req, &res);

    if (status.ok()) {
        if (res.result() != 0) {
            log_error("user_register failed!");
            return -1;
        }
        return 0;
    } else {
        log_error("Call to im server failed! err msg: %s", status.error_message().c_str());
        return -1;
    }
}

int call_im_server_grpc::login(const std::string& user_id,
                               const std::string& user_pass,
                               uint16_t client_port) {
    login_req req;
    req.set_user_id(user_id);
    req.set_user_pass(user_pass);
    req.set_client_port(client_port);

    login_res res;

    ClientContext context;
    Status status = m_stub->login(&context, req, &res);

    if (status.ok()) {
        if (res.result() != 0) {
            log_error("login failed!");
            return -1;
        }
        return 0;
    } else {
        log_error("Call to im server failed! err msg: %s", status.error_message().c_str());
        return -1;
    }
}

std::shared_ptr<prod_im_cont_list>
call_im_server_grpc::get_contact_list(const std::string& user_id) {
    get_contact_list_req req;
    req.set_user_id(user_id);

    get_contact_list_res res;

    ClientContext context;
    Status status = m_stub->get_contact_list(&context, req, &res);

    if (status.ok()) {
        if (res.result() != 0) {
            log_error("get_contact_list failed!");
            return nullptr;
        }
        auto ret_contact_list = std::make_shared<prod_im_cont_list>();
        auto& contact_list = res.contact_list();
        int idx;
        for (idx = 0; idx < contact_list.size(); idx++) {
            auto& a_contact = contact_list.at(idx);
            ret_contact_list->push_back(
                    {a_contact.contact_id(), a_contact.contact_name()});
        }
        return ret_contact_list;
    } else {
        log_error("Call to im server failed! err msg: %s", status.error_message().c_str());
        return nullptr;
    }
}

int call_im_server_grpc::add_contact(const std::string& user_id,
                                     const std::string& contact_id,
                                     const std::string& contact_name) {
    add_contact_req req;
    req.set_user_id(user_id);
    req.set_contact_id(contact_id);
    req.set_contact_name(contact_name);

    add_contact_res res;

    ClientContext context;
    Status status = m_stub->add_contact(&context, req, &res);

    if (status.ok()) {
        if (res.result() != 0) {
            log_error("add_contact failed!");
            return -1;
        }
        return 0;
    } else {
        log_error("Call to im server failed! err msg: %s", status.error_message().c_str());
        return -1;
    }
}

int call_im_server_grpc::del_contact(const std::string& user_id,
                                     const std::string& contact_id) {
    del_contact_req req;
    req.set_user_id(user_id);
    req.set_contact_id(contact_id);

    del_contact_res res;

    ClientContext context;
    Status status = m_stub->del_contact(&context, req, &res);

    if (status.ok()) {
        if (res.result() != 0) {
            log_error("del_contact failed!");
            return -1;
        }
        return 0;
    } else {
        log_error("Call to im server failed! err msg: %s", status.error_message().c_str());
        return -1;
    }
}

int call_im_server_grpc::client_send_chat_msg(const std::string& sender_id,
                                              const std::string& receiver_id,
                                              const std::string& chat_msg) {
    send_chat_msg_req req;
    req.set_sender_id(sender_id);
    req.set_receiver_id(receiver_id);
    req.set_chat_msg(chat_msg);

    send_chat_msg_res res;

    ClientContext context;
    Status status = m_stub->client_send_chat_msg(&context, req, &res);

    if (status.ok()) {
        if (res.result() != 0) {
            log_error("client_send_chat_msg failed!");
            return -1;
        }
        return 0;
    } else {
        log_error("Call to im server failed! err msg: %s", status.error_message().c_str());
        return -1;
    }
}

std::shared_ptr<prod_im_chat_msg_list> call_im_server_grpc::client_get_chat_msg(
        const std::string& user_id, size_t msg_index)
{
    get_chat_msg_req req;
    req.set_user_id(user_id);
    req.set_msg_index(msg_index);

    get_chat_msg_res res;

    ClientContext context;
    Status status = m_stub->client_get_chat_msg(&context, req, &res);

    if (status.ok()) {
        if (res.result() != 0) {
//            log_error("client_get_chat_msg failed!");
            return nullptr;
        }
        auto msg_list = std::make_shared<prod_im_chat_msg_list>();
        auto pb_msg_list = res.msg_list();
        auto msg = pb_msg_list.begin();
        for (; msg != pb_msg_list.end(); msg++) {
            msg_list->emplace_back(msg->sender_id(),
                                   msg->receiver_id(),
                                   msg->chat_msg());
        }
        return msg_list;
    } else {
//        log_error("Call to im server failed! err msg: %s", status.error_message().c_str());
        return nullptr;
    }
}
