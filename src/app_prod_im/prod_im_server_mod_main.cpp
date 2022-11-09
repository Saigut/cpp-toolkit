#include <thread>
#include <future>

#include <boost/asio/io_context.hpp>
#include <boost/asio/deadline_timer.hpp>

#include <mod_common/log.h>
#include <mod_common/expect.h>
#include <mod_coroutine/mod_coroutine.h>

#include "app_prod_im_internal.h"


/*
* 主模块
  * 注册
    register，参数：用户ID 字符串，用户密码 字符串；返回值：结果 int
  * 登录
    login，参数：用户ID 字符串，用户密码 字符串；返回值：结果 int
  * 获取联系人列表
    get_contact_list，参数：用户ID 字符串；返回值：联系人数组
  * 添加联系人
    add_contact，参数：用户ID 字符串，联系人ID 字符串，联系人名字 字符串；返回值：结果 int
  * 移除联系人
    del_contact，参数：用户ID 字符串，联系人ID 字符串；返回值：
  * 接收消息
    client_chat_msg，参数：发送者ID 字符串，接收者ID 字符串，聊天消息内容 字符串；返回值：结果 int
  * 运行
    run
*/

using boost::asio::deadline_timer;

int prod_im_s_mod_main::user_register(const std::string& user_id,
                                      const std::string& user_pass)
{
    int ret = -1;
    std::promise<int> prom;

    auto cb = [&](std::error_code ec){
        if (ec) {
            log_error("Register failed! User id: %s", user_id.c_str());
            prom.set_value(-1);
        } else {
            log_info("Register succeed! User id: %s", user_id.c_str());
            prom.set_value(0);
        }
    };
    prod_im_user_account user_acco = { user_id, user_pass };
    expect_ret_val(0 == user_register(user_acco, cb), -1);
    auto fut = prom.get_future();
    ret = fut.get();

    return ret;
}
int
prod_im_s_mod_main::login(const std::string& user_id, const std::string& user_pass, const std::string& client_ip,
                          uint16_t client_port)
{
    int ret = -1;
    std::promise<int> prom;

    auto cb = [&](std::error_code ec){
        if (ec) {
            log_error("Login failed! User id: %s", user_id.c_str());
            prom.set_value(-1);
        } else {
            log_info("Login succeed! User id: %s", user_id.c_str());
            prom.set_value(0);
        }
    };
    prod_im_user_account user_acco = { user_id, user_pass };
    prod_im_s_client_info client_info = { client_ip, client_port };
    expect_ret_val(0 == login(user_acco, client_info, cb), -1);
    auto fut = prom.get_future();
    ret = fut.get();

    return ret;
}
std::shared_ptr<prod_im_cont_list> prod_im_s_mod_main::get_contact_list(const std::string& user_id)
{
    int ret = -1;
    std::promise<int> prom;
    std::shared_ptr<prod_im_cont_list> rst = nullptr;

    auto cb = [&](std::error_code ec, std::shared_ptr<prod_im_cont_list> _cont_list){
        if (ec) {
            log_error("get_contact_list failed! User id: %s", user_id.c_str());
            prom.set_value(-1);
        } else {
            log_info("get_contact_list succeed! User id: %s", user_id.c_str());
            rst = _cont_list;
            prom.set_value(0);
        }
    };
    expect_ret_val(0 == get_contact_list(user_id, cb), nullptr);
    auto fut = prom.get_future();
    ret = fut.get();

    return rst;
}
int prod_im_s_mod_main::add_contact(const std::string& user_id,
                                    const std::string& contact_id,
                                    const std::string& contact_name)
{
    int ret = -1;
    std::promise<int> prom;

    auto cb = [&](std::error_code ec){
        if (ec) {
            log_error("add_contact failed! User id: %s", user_id.c_str());
            prom.set_value(-1);
        } else {
            log_info("add_contact succeed! User id: %s", user_id.c_str());
            prom.set_value(0);
        }
    };
    prod_im_contact cont = {contact_id, contact_name};
    expect_ret_val(0 == add_contact(user_id, cont, cb), -1);
    auto fut = prom.get_future();
    ret = fut.get();

    return ret;
}
int prod_im_s_mod_main::del_contact(const std::string& user_id,
                                    const std::string& contact_id)
{
    int ret = -1;
    std::promise<int> prom;

    auto cb = [&](std::error_code ec){
        if (ec) {
            log_error("del_contact failed! User id: %s", user_id.c_str());
            prom.set_value(-1);
        } else {
            log_info("del_contact succeed! User id: %s", user_id.c_str());
            prom.set_value(0);
        }
    };
    expect_ret_val(0 == del_contact(user_id, contact_id, cb), -1);
    auto fut = prom.get_future();
    ret = fut.get();

    return ret;
}
void prod_im_s_mod_main::client_chat_msg(const std::string& sender_id,
                                         const std::string& receiver_id,
                                         const std::string& chat_msg)
{
    int ret = -1;
    std::promise<int> prom;

    auto cb = [&](std::error_code ec){
        if (ec) {
            log_error("client_chat_msg failed! User id: %s", sender_id.c_str());
            prom.set_value(-1);
        } else {
            log_info("client_chat_msg succeed! User id: %s", sender_id.c_str());
            prom.set_value(0);
        }
    };
    prod_im_chat_msg st_chat_msg = {sender_id, receiver_id, chat_msg};
    expect_ret(0 == client_chat_msg(st_chat_msg, cb));
    auto fut = prom.get_future();
    ret = fut.get();
}
std::shared_ptr<prod_im_chat_msg_list> prod_im_s_mod_main::get_chat_msg(
        const std::string& user_id)
{
    int ret = -1;
    std::promise<int> prom;
    std::shared_ptr<prod_im_chat_msg_list> rst = nullptr;

    auto cb = [&](std::error_code ec, std::shared_ptr<prod_im_chat_msg_list> _msg_list){
        if (ec) {
            log_error("get_chat_msg failed! User id: %s", user_id.c_str());
            prom.set_value(-1);
        } else {
            log_info("get_chat_msg succeed! User id: %s", user_id.c_str());
            rst = _msg_list;
            prom.set_value(0);
        }
    };
    expect_ret_val(0 == get_chat_msg(user_id, cb), nullptr);
    auto fut = prom.get_future();
    ret = fut.get();

    return rst;
}

int prod_im_s_mod_main::user_register(const prod_im_user_account& user_acco, prod_im_s_main_common_cb&& cb)
{
    prod_im_s_mod_main_msg* msg = nullptr;

    msg = m_operation.alloc_msg();
    expect_goto(msg, fail_return);

    msg->type = emIM_S_MAIN_MSG_type_REG;
    msg->reg.use_acco = user_acco;
    msg->common_cb = std::move(cb);
    expect_goto(0 == m_operation.write_operation(msg), fail_return);

    return 0;

fail_return:
    if (msg) {
        m_operation.free_msg(msg);
    }
    return -1;
}

int prod_im_s_mod_main::login(const prod_im_user_account& user_acco, const prod_im_s_client_info& client_info,
                              prod_im_s_main_common_cb&& cb)
{
    prod_im_s_mod_main_msg* msg = nullptr;

    msg = m_operation.alloc_msg();
    expect_goto(msg, fail_return);

    msg->type = emIM_S_MAIN_MSG_type_LOGIN;
    msg->login.use_acco = user_acco;
    msg->login.client_info = client_info;
    msg->common_cb = cb;
    expect_goto(0 == m_operation.write_operation(msg), fail_return);

    return 0;

fail_return:
    if (msg) {
        m_operation.free_msg(msg);
    }
    return -1;
}

int prod_im_s_mod_main::get_contact_list(const std::string& user_id,
                                         prod_im_s_main_get_cont_list_cb&& cb)
{
    prod_im_s_mod_main_msg* msg = nullptr;

    msg = m_operation.alloc_msg();
    expect_goto(msg, fail_return);

    msg->type = emIM_S_MAIN_MSG_type_GET_CONT_LIST;
    msg->get_cont_list.user_id = user_id;
    msg->get_cont_list_cb = cb;
    expect_goto(0 == m_operation.write_operation(msg), fail_return);

    return 0;

fail_return:
    if (msg) {
        m_operation.free_msg(msg);
    }
    return -1;
}

int prod_im_s_mod_main::add_contact(const std::string& user_id,
                                    const prod_im_contact& cont,
                                    prod_im_s_main_common_cb&& cb)
{
    prod_im_s_mod_main_msg* msg = nullptr;

    msg = m_operation.alloc_msg();
    expect_goto(msg, fail_return);

    msg->type = emIM_S_MAIN_MSG_type_ADD_CONT;
    msg->add_cont.user_id = user_id;
    msg->add_cont.cont = cont;
    msg->common_cb = cb;
    expect_goto(0 == m_operation.write_operation(msg), fail_return);

    return 0;

fail_return:
    if (msg) {
        m_operation.free_msg(msg);
    }
    return -1;
}

int
prod_im_s_mod_main::del_contact(const std::string& user_id,
                                const std::string& contact_id,
                                prod_im_s_main_common_cb&& cb)
{
    prod_im_s_mod_main_msg* msg = nullptr;

    msg = m_operation.alloc_msg();
    expect_goto(msg, fail_return);

    msg->type = emIM_S_MAIN_MSG_type_DEL_CONT;
    msg->del_cont.user_id = user_id;
    msg->del_cont.cont_id = contact_id;
    msg->common_cb = cb;
    expect_goto(0 == m_operation.write_operation(msg), fail_return);

    return 0;

fail_return:
    if (msg) {
        m_operation.free_msg(msg);
    }
    return -1;
}

int prod_im_s_mod_main::client_chat_msg(prod_im_chat_msg& chat_msg,
                                        prod_im_s_main_common_cb&& cb)
{
    prod_im_s_mod_main_msg* msg = nullptr;

    msg = m_operation.alloc_msg();
    expect_goto(msg, fail_return);

    msg->type = emIM_S_MAIN_MSG_type_CLIENT_MSG;
    msg->client_msg.chat_msg = chat_msg;
    msg->common_cb = cb;
    expect_goto(0 == m_operation.write_operation(msg), fail_return);

    return 0;

fail_return:
    if (msg) {
        m_operation.free_msg(msg);
    }
    return -1;
}

int prod_im_s_mod_main::get_chat_msg(const std::string& user_id,
                                     prod_im_s_main_get_chat_msg_cb&& cb)
{
    return -1;
}


int prod_im_s_mod_main_operation::write_operation(prod_im_s_mod_main_msg* msg)
{
    if (m_op_msg_notification_on) {
        std::lock_guard lock(op_writer_lock);
        expect_ret_val(0 == m_op_queue.push(msg), -1);
        if (notify_to_read_op_func) {
            auto temp_f = std::move(notify_to_read_op_func);
            notify_to_read_op_func = nullptr;
            // todo: think of checking result
            temp_f();
        }
    } else {
        expect_ret_val(0 == m_op_queue.push(msg), -1);
    }
    return 0;
}

void prod_im_s_mod_main_operation::read_operation()
{
    m_op_msg_notification_on = false;

    while (!m_op_queue.empty()) {
        void* p = nullptr;
        m_op_queue.pop(p);
        auto msg = (prod_im_s_mod_main_msg*)p;
        if (msg) {
            process_operation(msg);
            free_msg(msg);
        }
    }

    auto wrap_func = [&](std::function<void()>&& co_cb) {
        m_op_msg_notification_on = true;
        {
            std::lock_guard lock(op_writer_lock);
            if (!m_op_queue.empty()) {
                // todo: think of checking result
                co_cb();
            } else {
                notify_to_read_op_func = co_cb;
            }
        }
    };
    cppt_co_yield(wrap_func);
}

int prod_im_s_mod_main_operation::process_operation(prod_im_s_mod_main_msg* msg)
{
    switch (msg->type) {
        case emIM_S_MAIN_MSG_type_REG: {
            if (0 == user_register(msg->reg.use_acco)) {
                msg->common_cb(std::error_code());
            } else {
                msg->common_cb(
                        std::make_error_code(std::errc::interrupted));
            }
            break;
        }
        case emIM_S_MAIN_MSG_type_LOGIN: {
            if (0 == login(msg->login.use_acco, msg->login.client_info)) {
                msg->common_cb(std::error_code());
            } else {
                msg->common_cb(
                        std::make_error_code(std::errc::interrupted));
            }
            break;
        }
        case emIM_S_MAIN_MSG_type_GET_CONT_LIST: {
            auto ret_list = get_contact_list(msg->get_cont_list.user_id);
            if (ret_list) {
                msg->get_cont_list_cb(std::error_code(), ret_list);
            } else {
                msg->get_cont_list_cb(
                        std::make_error_code(std::errc::interrupted), nullptr);
            }
            break;
        }
        case emIM_S_MAIN_MSG_type_ADD_CONT: {
            if (0 == add_contact(msg->add_cont.user_id, msg->add_cont.cont)) {
                msg->common_cb(std::error_code());
            } else {
                msg->common_cb(
                        std::make_error_code(std::errc::interrupted));
            }
            break;
        }
        case emIM_S_MAIN_MSG_type_DEL_CONT: {
            if (0 == del_contact(msg->del_cont.user_id, msg->del_cont.cont_id)) {
                msg->common_cb(std::error_code());
            } else {
                msg->common_cb(
                        std::make_error_code(std::errc::interrupted));
            }
            break;
        }
        case emIM_S_MAIN_MSG_type_CLIENT_MSG: {
            if (0 == client_chat_msg(msg->client_msg.chat_msg)) {
                msg->common_cb(std::error_code());
            } else {
                msg->common_cb(
                        std::make_error_code(std::errc::interrupted));
            }
            break;
        }

        default: {
            return -1;
        }
    }
    return 0;
}

prod_im_s_mod_main_msg * prod_im_s_mod_main_operation::alloc_msg()
{
    void* p = nullptr;
    prod_im_s_mod_main_msg* msg = nullptr;

    p = m_op_mpool.alloc();
    expect_goto(p, fail_return);
    msg = new(p) prod_im_s_mod_main_msg;
    expect_goto(msg, fail_return);

    return msg;

fail_return:
    if (p) {
        m_op_mpool.free(p);
    }
    return nullptr;
}

void prod_im_s_mod_main_operation::free_msg(prod_im_s_mod_main_msg* msg)
{
    expect_ret(msg);
    m_op_mpool.free(msg);
}

int
prod_im_s_mod_main_operation::user_register(const prod_im_user_account& user_acco)
{
    return m_user_info.user_add(user_acco.user_id, user_acco.user_pass);
}

int prod_im_s_mod_main_operation::login(const prod_im_user_account& user_acco,
                                        const prod_im_s_client_info& client_info)
{
    auto& user_id = user_acco.user_id;
    auto find_rst = m_user_info.user_find(user_id);
    if (!find_rst) {
        log_error("No such user! User id: %s", user_id.c_str());
        return -1;
    }
    if (user_acco.user_pass != find_rst->user_pass) {
        log_error("Wrong password! User id: %s", user_id.c_str());
        return -1;
    }
    if (0 != m_user_session.add(user_id, client_info.client_ip, client_info.client_port)) {
        log_error("Failed to create user session! User id: %s", user_id.c_str());
        return -1;
    }
//    deadline_timer timer{m_io_ctx, boost::posix_time::seconds(50)};
//    timer.async_wait([user_id, v_this(this)](const boost::system::error_code& e){
//        v_this->m_user_session.del(user_id);
//    });
    return 0;
}

std::shared_ptr<prod_im_cont_list>
prod_im_s_mod_main_operation::get_contact_list(const std::string& user_id)
{
    auto find_rst = m_user_info.user_find(user_id);
    if (!find_rst) {
        log_error("User is not registered! User id: %s", user_id.c_str());
        return nullptr;
    }
    auto rst = m_user_session.find(user_id);
    if (!rst) {
        log_error("User is not login! User id: %s", user_id.c_str());
        return nullptr;
    }
    return m_user_info.user_contact_get_list(user_id);
}

int prod_im_s_mod_main_operation::add_contact(const std::string& user_id,
                                              const prod_im_contact& cont)
{
    auto find_rst = m_user_info.user_find(user_id);
    if (!find_rst) {
        log_error("User is not registered! User id: %s", user_id.c_str());
        return -1;
    }
    auto rst = m_user_session.find(user_id);
    if (!rst) {
        log_error("User is not login! User id: %s", user_id.c_str());
        return -1;
    }
    return m_user_info.user_contact_add(user_id, cont.contact_id, cont.contact_name);
}

int prod_im_s_mod_main_operation::del_contact(const std::string& user_id,
                                              const std::string& contact_id)
{
    auto find_rst = m_user_info.user_find(user_id);
    if (!find_rst) {
        log_error("User is not registered! User id: %s", user_id.c_str());
        return -1;
    }
    auto rst = m_user_session.find(user_id);
    if (!rst) {
        log_error("User does not login! User id: %s", user_id.c_str());
        return -1;
    }
    m_user_info.user_contact_del(user_id, contact_id);
    return 0;
}

int prod_im_s_mod_main_operation::client_chat_msg(prod_im_chat_msg& chat_msg)
{
    auto& sender_id = chat_msg.sender_id;
    auto& receiver_id = chat_msg.receiver_id;
    auto& chat_msg_text = chat_msg.chat_msg;
    auto rst = m_user_session.find(sender_id);
    if (!rst) {
        log_error("User does not login! User id: %s", sender_id.c_str());
        return -1;
    }
    rst = m_user_session.find(receiver_id);
    if (!rst) {
        log_error("Receiver does not login! Receiver id: %s", receiver_id.c_str());
        return -1;
    }
    m_chat_msg_relay.relay_msg(rst->client_ip, rst->client_port,
                               sender_id, receiver_id,
                               chat_msg_text);
}

void prod_im_s_mod_main::run()
{
    auto co_im_s_main = [&](){
        while (true) {
            m_operation.read_operation();
        }
    };
    cppt_co_create(co_im_s_main);
    cppt_co_main_run();
}
