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

int prod_im_s_mod_uinfo::user_add(std::string& user_id, std::string& user_pass)
{
    return -1;
}

void prod_im_s_mod_uinfo::user_del(std::string& user_id)
{

}

int prod_im_s_mod_uinfo::user_contact_add(std::string& user_id,
                                     std::string& contact_id,
                                     std::string& contact_name)
{
    return -1;
}

void prod_im_s_mod_uinfo::user_contact_del(std::string& user_id, std::string& contact_id)
{

}

std::vector<prod_im_contact>&& prod_im_s_mod_uinfo::user_contact_get_list(std::string& user_id)
{
    return std::move(std::vector<prod_im_contact>{});
}