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
    return m_user_info.user_add(user_id, user_pass);
}
int
prod_im_s_mod_main::login(const std::string& user_id, const std::string& user_pass, const std::string& client_ip,
                          uint16_t client_port)
{
    auto find_rst = m_user_info.user_find(user_id);
    if (!find_rst) {
        log_error("No such user! User id: %s", user_id.c_str());
        return -1;
    }
    if (user_pass != find_rst->user_pass) {
        log_error("Wrong password! User id: %s", user_id.c_str());
        return -1;
    }
    if (0 != m_user_session.add(user_id, client_ip, client_port)) {
        log_error("Failed to create user session! User id: %s", user_id.c_str());
        return -1;
    }
//    deadline_timer timer{m_io_ctx, boost::posix_time::seconds(50)};
//    timer.async_wait([user_id, v_this(this)](const boost::system::error_code& e){
//        v_this->m_user_session.del(user_id);
//    });
    return 0;
}
std::shared_ptr<prod_im_cont_list> prod_im_s_mod_main::get_contact_list(const std::string& user_id)
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
int prod_im_s_mod_main::add_contact(const std::string& user_id,
                                    const std::string& contact_id,
                                    const std::string& contact_name)
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
    return m_user_info.user_contact_add(user_id, contact_id, contact_name);
}
int prod_im_s_mod_main::del_contact(const std::string& user_id,
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
void prod_im_s_mod_main::client_chat_msg(const std::string& sender_id,
                                         const std::string& receiver_id,
                                         const std::string& chat_msg)
{
    auto rst = m_user_session.find(sender_id);
    if (!rst) {
        log_error("User does not login! User id: %s", sender_id.c_str());
        return;
    }
    rst = m_user_session.find(receiver_id);
    if (!rst) {
        log_error("Receiver does not login! Receiver id: %s", receiver_id.c_str());
        return;
    }
    m_chat_msg_relay.relay_msg(rst->client_ip, rst->client_port, sender_id, receiver_id, chat_msg);
}

int prod_im_s_mod_main::user_register(const prod_im_user_account& user_acco, prod_im_s_main_common_cb&& cb)
{
    void* p = nullptr;
    prod_im_s_mod_main_msg* msg = nullptr;

    p = m_operation_mpool.alloc();
    expect_goto(p, fail_return);
    msg = new(p) prod_im_s_mod_main_msg;
    expect_goto(msg, fail_return);

    msg->type = emIM_S_MAIN_MSG_type_REG;
    msg->reg.use_acco = user_acco;
    msg->common_cb = std::move(cb);
    expect_goto(0 == m_operation_queue.push(msg), fail_return);

    return 0;

fail_return:
    if (p) {
        m_operation_mpool.free(msg);
    }
    return -1;
}

int prod_im_s_mod_main::login(const prod_im_user_account& user_acco, const prod_im_s_client_info& client_info,
                              prod_im_s_main_common_cb&& cb)
{
    void* p = nullptr;
    prod_im_s_mod_main_msg* msg = nullptr;

    p = m_operation_mpool.alloc();
    expect_goto(p, fail_return);
    msg = new(p) prod_im_s_mod_main_msg;
    expect_goto(msg, fail_return);

    msg->type = emIM_S_MAIN_MSG_type_LOGIN;
    msg->login.use_acco = user_acco;
    msg->login.client_info = client_info;
    msg->common_cb = cb;
    expect_goto(0 == m_operation_queue.push(msg), fail_return);

    return 0;

fail_return:
    if (p) {
        m_operation_mpool.free(msg);
    }
    return -1;
}

int prod_im_s_mod_main::get_contact_list(const std::string& user_id,
                                         prod_im_s_main_get_cont_list_cb&& cb)
{
    void* p = nullptr;
    prod_im_s_mod_main_msg* msg = nullptr;

    p = m_operation_mpool.alloc();
    expect_goto(p, fail_return);
    msg = new(p) prod_im_s_mod_main_msg;
    expect_goto(msg, fail_return);

    msg->type = emIM_S_MAIN_MSG_type_GET_CONT_LIST;
    msg->get_cont_list.user_id = user_id;
    msg->get_cont_list_cb = cb;
    expect_goto(0 == m_operation_queue.push(msg), fail_return);

    return 0;

fail_return:
    if (p) {
        m_operation_mpool.free(msg);
    }
    return -1;
}

int prod_im_s_mod_main::add_contact(const std::string& user_id,
                                    const prod_im_contact& cont,
                                    prod_im_s_main_common_cb&& cb)
{
    void* p = nullptr;
    prod_im_s_mod_main_msg* msg = nullptr;

    p = m_operation_mpool.alloc();
    expect_goto(p, fail_return);
    msg = new(p) prod_im_s_mod_main_msg;
    expect_goto(msg, fail_return);

    msg->type = emIM_S_MAIN_MSG_type_ADD_CONT;
    msg->add_cont.user_id = user_id;
    msg->add_cont.cont = cont;
    msg->common_cb = cb;
    expect_goto(0 == m_operation_queue.push(msg), fail_return);

    return 0;

fail_return:
    if (p) {
        m_operation_mpool.free(msg);
    }
    return -1;
}

int
prod_im_s_mod_main::del_contact(const std::string& user_id,
                                const std::string& contact_id,
                                prod_im_s_main_common_cb&& cb)
{
    void* p = nullptr;
    prod_im_s_mod_main_msg* msg = nullptr;

    p = m_operation_mpool.alloc();
    expect_goto(p, fail_return);
    msg = new(p) prod_im_s_mod_main_msg;
    expect_goto(msg, fail_return);

    msg->type = emIM_S_MAIN_MSG_type_DEL_CONT;
    msg->del_cont.user_id = user_id;
    msg->del_cont.cont_id = contact_id;
    msg->common_cb = cb;
    expect_goto(0 == m_operation_queue.push(msg), fail_return);

    return 0;

fail_return:
    if (p) {
        m_operation_mpool.free(msg);
    }
    return -1;
}

int prod_im_s_mod_main::client_chat_msg(prod_im_chat_msg& chat_msg,
                                        prod_im_s_main_common_cb&& cb)
{
    void* p = nullptr;
    prod_im_s_mod_main_msg* msg = nullptr;

    p = m_operation_mpool.alloc();
    expect_goto(p, fail_return);
    msg = new(p) prod_im_s_mod_main_msg;
    expect_goto(msg, fail_return);

    msg->type = emIM_S_MAIN_MSG_type_CLIENT_MSG;
    msg->client_msg.chat_msg = chat_msg;
    msg->common_cb = cb;
    expect_goto(0 == m_operation_queue.push(msg), fail_return);

    return 0;

fail_return:
    if (p) {
        m_operation_mpool.free(msg);
    }
    return -1;
}

void prod_im_s_mod_main::notify_reader()
{
    if (!reader_is_idle) {
        return;
    }
    if (notified_reader) {
        return;
    }
    if (!notify_lock.try_lock()) {
        return;
    }
    // todo  push to coroutine queue!
    notified_reader = true;
    notify_lock.unlock();
}


int prod_im_s_mod_main::writer_write(void* msg)
{
    expect_ret_val(0 == m_operation_queue.push(msg), -1);
    if (m_notification_on) {
        m_notification_indeed_on = true;
        if (notify_func) {
            notify_func();
        }
    } else {

    }

}

void prod_im_s_mod_main::reader_read()
{
    m_notification_on = false;
//    reader_is_idle = false;

    while (!m_operation_queue.empty()) {
        void* p;
        m_operation_queue.pop(p);
        // todo  process msg
    }

    notify_func = [](){};
    m_notification_on = true;
    while (!m_notification_indeed_on && !m_operation_queue.empty()) {
        void* p;
        m_operation_queue.pop(p);
        // todo  process msg
    }
}

void cor_main(prod_im_s_mod_main& obj)
{

}

void prod_im_s_mod_main::run()
{

    cppt_co_create(cor_main, std::ref(*this));

    cppt_co_main_run();
}
