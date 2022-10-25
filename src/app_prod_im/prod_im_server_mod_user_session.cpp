#include "app_prod_im_internal.h"

#include <mod_common/log.h>
#include <boost/asio/deadline_timer.hpp>


/*
* 用户会话
  * 添加用户会话
    user_session_add，参数：用户ID 字符串，用户会话；返回值：结果 int
  * 移除用户会话
    user_session_del，参数：用户ID 字符串
  * 查找用户会话
    user_session_find，参数：用户ID 字符串；返回值：用户会话
*/

using boost::asio::deadline_timer;

int prod_im_s_mod_user_session::add(std::string& user_id, int io_port)
{
    auto rst = m_user_sessions.find(user_id);
    if (rst != m_user_sessions.end()) {
        log_error("User session existed! User id: %s", user_id.c_str());
        return -1;
    }
    auto insert_rst = m_user_sessions.insert({user_id,
                                              prod_im_s_user_session{user_id,
                                                                     io_port,
                                                                     boost::asio::deadline_timer(m_io_ctx)}});
    if (!insert_rst.second) {
        log_error("Failed to insert user session! User id: %s", user_id.c_str());
        return -1;
    }
    return 0;
}

void prod_im_s_mod_user_session::del(const std::string& user_id)
{
    m_user_sessions.erase(user_id);
}

std::shared_ptr<prod_im_s_user_session> prod_im_s_mod_user_session::find(std::string& user_id)
{
    auto rst = m_user_sessions.find(user_id);
    if (rst == m_user_sessions.end()) {
        return nullptr;
    }
    return std::make_shared<prod_im_s_user_session>(rst->second);
}
