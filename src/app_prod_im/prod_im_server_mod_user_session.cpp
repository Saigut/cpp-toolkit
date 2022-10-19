#include "app_prod_im_internal.h"


/*
* 用户会话
  * 添加用户会话
    user_session_add，参数：用户ID 字符串，用户会话；返回值：结果 int
  * 移除用户会话
    user_session_del，参数：用户ID 字符串
  * 查找用户会话
    user_session_find，参数：用户ID 字符串；返回值：用户会话
*/

int prod_im_s_mod_user_session::add(std::string& user_id)
{
    return -1;
}

void prod_im_s_mod_user_session::del(std::string& user_id)
{

}

prod_im_s_user_session&& prod_im_s_mod_user_session::find(std::string& user_id)
{
    return std::move(prod_im_s_user_session{});
}
