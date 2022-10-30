#include "app_prod_im_internal.h"



/*
* 聊天消息发送
  * 发送聊天消息
    send_msg，参数：接收者ID 字符串；消息内容 字符串；返回值：结果 int
*/

int
prod_im_c_chat_msg_sending::send_msg(const std::string& server_ip,
                                     uint16_t server_port,
                                     const std::string& sender_id,
                                     const std::string& receiver_id,
                                     const std::string& msg_content)
{
    std::string im_server_listen_addr = server_ip + ":" + std::to_string(server_port);
    call_im_server_grpc call_to_im_server(grpc::CreateChannel(im_server_listen_addr,
                                                       grpc::InsecureChannelCredentials()));
    return call_to_im_server.send_chat_msg(sender_id, receiver_id, msg_content);
}
