#include "prod_im_server_mod_user_info.hpp"

#include <mod_common/log.h>
#include <algorithm>
#include <iterator>

/*
* 用户信息
  * 添加用户
    user_add，参数：用户ID 字符串，用户密码 字符串；返回值：结果 int
  * 移除用户
    user_del，参数：用户ID 字符串
  * 添加用户联系人
    user_contact_add，参数：用户ID 字符串，联系人ID 字符串，联系人备注 字符串；返回值：结果 int
  * 移除用户联系人
    user_contact_del，参数：用户ID 字符串，联系人ID 字符串
  * 获取用户联系人列表
    user_contact_get_list，参数：用户ID 字符串；返回值：联系人列表 联系人ID+联系人备注
*/

int prod_im_s_mod_uinfo::user_add(const std::string& user_id, const std::string& user_pass)
{
    auto rst = m_users.find(user_id);
    if (rst != m_users.end()) {
        log_error("User existed! Id: %s", user_id.c_str());
        return -1;
    }
    auto insert_rst = m_users.insert({user_id, {user_id, user_pass}});
    if (!insert_rst.second) {
        log_error("Failed to add user! Id: %s", user_id.c_str());
        return -1;
    }
    return 0;
}

void prod_im_s_mod_uinfo::user_del(const std::string& user_id)
{
    m_users.erase(user_id);
}

std::shared_ptr<prod_im_s_mod_uinfo::user_info_t> prod_im_s_mod_uinfo::user_find(const std::string& user_id)
{
    auto rst = m_users.find(user_id);
    if (rst == m_users.end()) {
        return nullptr;
    }
    return std::make_shared<prod_im_s_mod_uinfo::user_info_t>(rst->second);
}

bool prod_im_s_mod_uinfo::user_exist(const std::string& user_id)
{
    auto rst = m_users.find(user_id);
    return (rst != m_users.end());
}

int prod_im_s_mod_uinfo::user_get_chat_msg(const std::string& user_id,
                                           prod_im_chat_msg_list& chat_msg)
{
    auto rst = m_users.find(user_id);
    if (rst == m_users.end()) {
        return -2;
    }
    auto& u_msg = rst->second.user_chat_msg;
    if (u_msg.empty()) {
        return 0;
    }
    chat_msg.insert(chat_msg.end(),
                    std::make_move_iterator(u_msg.begin()),
                    std::make_move_iterator(u_msg.end()));
    u_msg.clear();
    return 0;
}

int prod_im_s_mod_uinfo::user_get_chat_msg_from(const std::string& user_id,
                                                size_t msg_index,
                                                prod_im_chat_msg_list& chat_msg)
{
    auto rst = m_users.find(user_id);
    if (rst == m_users.end()) {
        return -2;
    }
    auto& u_msg = rst->second.user_chat_msg;
    if (u_msg.empty()) {
        return 0;
    }
    for (; msg_index < u_msg.size(); msg_index++) {
        chat_msg.push_back(u_msg[msg_index]);
    }
    return 0;
}

int prod_im_s_mod_uinfo::user_add_msg(const std::string& user_id, prod_im_chat_msg& chat_msg)
{
    auto rst = m_users.find(user_id);
    if (rst == m_users.end()) {
        return -2;
    }
    auto& u_msg = rst->second.user_chat_msg;
    u_msg.push_back(chat_msg);
    return 0;
}

int prod_im_s_mod_uinfo::user_contact_add(const std::string& user_id,
                                          const std::string& contact_id,
                                          const std::string& contact_name)
{
    auto rst = m_users.find(user_id);
    if (rst == m_users.end()) {
        log_error("User not existed! Id: %s", user_id.c_str());
        return -1;
    }
    auto& contacts = rst->second.user_contacts;
    auto find_rst = contacts.find(contact_id);
    if (find_rst != contacts.end()) {
        find_rst->second.contact_id = contact_id;
        find_rst->second.contact_name = contact_name;
    } else {
        auto insert_rst = contacts.insert({contact_id, {contact_id, contact_name}});
        if (!insert_rst.second) {
            log_error("Failed to add contact! User id: %s, contact: %s %s",
                      user_id.c_str(), contact_id.c_str(), contact_name.c_str());
            return -1;
        }
    }
    return 0;
}

void prod_im_s_mod_uinfo::user_contact_del(const std::string& user_id, const std::string& contact_id)
{
    auto rst = m_users.find(user_id);
    if (rst == m_users.end()) {
        log_error("User not existed! Id: %s", user_id.c_str());
        return;
    }
    rst->second.user_contacts.erase(contact_id);
}

std::shared_ptr<prod_im_cont_list> prod_im_s_mod_uinfo::user_contact_get_list(const std::string& user_id)
{
    auto rst = m_users.find(user_id);
    if (rst == m_users.end()) {
        return nullptr;
    }
    auto& contacts = rst->second.user_contacts;

    auto contact_list = std::make_shared<prod_im_cont_list>();
    std::transform(contacts.begin(), contacts.end(),
                   std::back_inserter(*contact_list),
                   [](const std::pair<std::string, prod_im_contact> &p) { return p.second; });
    return contact_list;
}
