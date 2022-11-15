#include "prod_im_client_mod_contacts.hpp"



/*
* 联系人管理
  * 更新联系人列表
    update_list，参数：联系人列表 vector数组 引用
  * 添加联系人
    add_contact，参数：联系人ID 字符串；联系人备注 字符串；返回值：结果 int
  * 移除联系人
    del_contact，参数：联系人ID 字符串
*/

void prod_im_c_mod_contacts::update_list(prod_im_cont_list& contact_list)
{
    m_contacts.clear();
    for (auto& item : contact_list) {
        m_contacts.insert({item.contact_id,
                           {item.contact_id, item.contact_name}});
    }
}

int prod_im_c_mod_contacts::add_contact(std::string& contact_id, std::string& contact_name)
{
    auto rst = m_contacts.find(contact_id);
    if (rst != m_contacts.end()) {
        rst->second.contact_id = contact_id;
        rst->second.contact_name = contact_name;
    } else {
        m_contacts.insert({contact_id, {contact_id, contact_name}});
    }
    return 0;
}

int prod_im_c_mod_contacts::del_contact(std::string& contact_id)
{
    m_contacts.erase(contact_id);
    return 0;
}
