#include <mod_common/log.h>
#include <boost/asio/io_context.hpp>
#include <boost/asio/deadline_timer.hpp>
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
    recv_chat_msg，参数：发送者ID 字符串，接收者ID 字符串，聊天消息内容 字符串；返回值：结果 int
  * 运行
    run
*/

using boost::asio::deadline_timer;

int prod_im_s_mod_main::user_register(std::string& user_id,
                  std::string& user_pass)
{
    return m_user_info.user_add(user_id, user_pass);
}
int
prod_im_s_mod_main::login(std::string& user_id, std::string& user_pass, int io_port)
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
    if (0 != m_user_session.add(user_id, io_port)) {
        log_error("Failed to create user session! User id: %s", user_id.c_str());
        return -1;
    }
    deadline_timer timer{m_io_ctx, boost::posix_time::seconds(50)};
    timer.async_wait([user_id, v_this(this)](const boost::system::error_code& e){
        v_this->m_user_session.del(user_id);
    });
    return 0;
}
std::vector<prod_im_contact>&& prod_im_s_mod_main::get_contact_list(std::string& user_id)
{
    auto find_rst = m_user_info.user_find(user_id);
    if (!find_rst) {
        log_error("User is not registered! User id: %s", user_id.c_str());
        return std::move(std::vector<prod_im_contact>{});
    }
    auto rst = m_user_session.find(user_id);
    if (!rst) {
        log_error("User is not login! User id: %s", user_id.c_str());
        return std::move(std::vector<prod_im_contact>{});
    }
    return  m_user_info.user_contact_get_list(user_id);
}
int prod_im_s_mod_main::add_contact(std::string& user_id,
                std::string& contact_id,
                std::string& contact_name)
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
int prod_im_s_mod_main::del_contact(std::string& user_id,
                                    std::string& contact_id)
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
    m_user_info.user_contact_del(user_id, contact_id);
    return 0;
}
void prod_im_s_mod_main::recv_chat_msg(std::string& sender_id,
                   std::string& receiver_id,
                   std::string& chat_msg)
{
    m_chat_msg_relay.relay_msg(sender_id, receiver_id, chat_msg);
}
void prod_im_s_mod_main::run()
{
    boost::asio::io_context::work io_work(m_io_ctx);
    m_io_ctx.run();
}