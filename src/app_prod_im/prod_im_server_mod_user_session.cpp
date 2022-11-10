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

int prod_im_s_mod_user_session::add(const std::string& user_id, const std::string& client_ip, uint16_t client_port)
{
    auto rst = m_user_sessions.find(user_id);
    if (rst != m_user_sessions.end()) {
        auto& session = rst->second;
        session.user_id = user_id;
        session.client_ip = client_ip;
        session.client_port = client_port;
        return 0;
    }
    auto insert_rst = m_user_sessions.insert(
            {user_id,
             prod_im_s_user_session{user_id, client_ip, client_port,
                     std::make_shared<boost::asio::deadline_timer>(m_io_ctx)}});
    if (!insert_rst.second) {
        log_error("Failed to insert user session! User id: %s", user_id.c_str());
        return -1;
    }
//    auto& timer = insert_rst.first->second.timer;
//    timer->expires_from_now(boost::posix_time::seconds(300));
//    timer->async_wait([user_id, v_this(this)](const boost::system::error_code& ec){
//        log_info("ec: %s", ec.message().c_str());
//        if (ec.value() == boost::system::errc::timed_out) {
//            log_warn("Session timed out! User id: %s", user_id.c_str());
//            v_this->del(user_id);
//        }
//    });
    auto find_rst = m_user_sessions.find(user_id);
    if (find_rst == m_user_sessions.end()) {
        log_error("Failed to create user session! User id: %s", user_id.c_str());
    } else {
        log_info("Succeed to create user session! User id: %s", user_id.c_str());
    }
    return 0;
}

void prod_im_s_mod_user_session::del(const std::string& user_id)
{
    m_user_sessions.erase(user_id);
}

std::shared_ptr<prod_im_s_user_session> prod_im_s_mod_user_session::find(const std::string& user_id)
{
    auto rst = m_user_sessions.find(user_id);
    if (rst == m_user_sessions.end()) {
//        log_error("No such user session! User id: %s", user_id.c_str());
        return nullptr;
    }
    auto& u_session = rst->second;
//    auto& timer = u_session.timer;
//    timer->cancel();
//    timer->expires_from_now(boost::posix_time::seconds(300));
//    timer->async_wait([user_id, v_this(this)](const boost::system::error_code& ec){
//        log_info("ec: %s", ec.message().c_str());
//        if (ec.value() == boost::system::errc::timed_out) {
//            log_warn("Session timed out! User id: %s", user_id.c_str());
//            v_this->del(user_id);
//        }
//    });
    return std::make_shared<prod_im_s_user_session>(u_session.user_id,
                                                    u_session.client_ip,
                                                    u_session.client_port,
                                                    u_session.timer);
}
