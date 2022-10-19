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

int prod_im_s_mod_main::user_register(std::string& user_id,
                  std::string& user_pass)
{
    return -1;
}
int prod_im_s_mod_main::login(std::string& user_id,
          std::string& user_pass)
{
    return -1;
}
std::vector<prod_im_contact>&& prod_im_s_mod_main::get_contact_list(std::string& user_id)
{
    return std::move(std::vector<prod_im_contact>{});
}
int prod_im_s_mod_main::add_contact(std::string& user_id,
                std::string& contact_id,
                std::string& contact_name)
{
    return -1;
}
void prod_im_s_mod_main::del_contact(std::string& user_id,
                 std::string& contact_id)
{

}
void prod_im_s_mod_main::recv_chat_msg(std::string& sender_id,
                   std::string& receiver_id,
                   std::string& chat_msg)
{

}
void prod_im_s_mod_main::run()
{

}