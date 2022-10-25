#include <mod_common/log.h>
#include <algorithm>
#include "app_prod_im_internal.h"

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

std::vector<prod_im_contact>&& prod_im_s_mod_uinfo::user_contact_get_list(const std::string& user_id)
{
    auto rst = m_users.find(user_id);
    if (rst == m_users.end()) {
        return std::move(std::vector<prod_im_contact>{});
    }
    auto& contacts = rst->second.user_contacts;

    std::vector<prod_im_contact> contact_list;
    std::transform(contacts.begin(), contacts.end(),
                   std::back_inserter(contact_list),
                   [](const std::pair<std::string, prod_im_contact> &p) { return p.second; });
    return std::move(contact_list);
}
