#include "app_prod_im_internal.h"


/*
* 主模块
  * 注册
    register，参数：用户ID 字符串，用户密码 字符串；返回值：结果 int
  * 登录
    login，参数：用户ID 字符串，用户密码 字符串；返回值：结果 int
  * 获取联系人列表
    get_contact_list，参数：无；返回值：联系人数组
  * 添加联系人
    add_contact，参数：联系人ID 字符串，联系人名字 字符串；返回值：结果 int
  * 移除联系人
    del_contact，参数：联系人ID 字符串
  * 向联系人发送消息
    send_msg_to_contact，参数：联系人ID 字符串，聊天消息内容 字符串；返回值：结果 int
  * 接收聊天消息
    client_chat_msg，参数：发送者ID 字符串，聊天消息内容
  * 运行
    run
*/

int prod_im_c_mod_main::user_register(const std::string& user_pass)
{
    return m_server_grpc_api->user_register(m_my_id, user_pass);
}
int prod_im_c_mod_main::login(const std::string& user_pass)
{
    return m_server_grpc_api->login(m_my_id, user_pass, m_client_port);
}
std::shared_ptr<prod_im_cont_list> prod_im_c_mod_main::get_contact_list()
{
    return m_server_grpc_api->get_contact_list(m_my_id);
}
int prod_im_c_mod_main::add_contact(const std::string& contact_id,
                                    const std::string& contact_name)
{
    return m_server_grpc_api->add_contact(m_my_id, contact_id, contact_name);
}
void prod_im_c_mod_main::del_contact(const std::string& contact_id)
{
    m_server_grpc_api->del_contact(m_my_id, contact_id);
}
int prod_im_c_mod_main::send_msg_to_contact(const std::string& contact_id,
                                            const std::string& chat_msg)
{
    return m_server_grpc_api->send_chat_msg(m_my_id, contact_id, chat_msg);
}
void prod_im_c_mod_main::client_chat_msg(const std::string& sender_id,
                                         const std::string& chat_msg)
{
//    printf("%s: %s\n", sender_id.c_str(), chat_msg.c_str());
    prod_im_client_mod_cli_recv_msg("%s: %s\n", sender_id.c_str(), chat_msg.c_str());
}
std::shared_ptr<prod_im_chat_msg_list> prod_im_c_mod_main::get_chat_msg()
{
    auto rst = m_server_grpc_api->get_chat_msg(m_my_id, m_msg_index);
    if (rst) {
        m_msg_index += rst->size();
    }
    return rst;
}
void prod_im_c_mod_main::run()
{

}
