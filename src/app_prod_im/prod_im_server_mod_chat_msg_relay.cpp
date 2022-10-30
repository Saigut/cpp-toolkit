#include "app_prod_im_internal.h"

#include <grpcpp/grpcpp.h>
#include <prod_im_client.grpc.pb.h>
#include <mod_common/log.h>

/*
* 聊天消息转发
  * 转发聊天消息
    msg_relay，参数：发送者ID 字符串，接收者ID 字符串，聊天消息内容 字符串；返回值：结果 int
*/

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;
using prod_im_client::prod_im_client_service;
using prod_im_client::send_chat_msg_req;
using prod_im_client::send_chat_msg_res;


class CallImClient {
public:
    CallImClient(std::shared_ptr<Channel> channel)
            : m_stub(prod_im_client_service::NewStub(channel)) {}

    int SendMsg(const std::string& sender_id,
                const std::string& receiver_id,
                const std::string& chat_content) {
        send_chat_msg_req request;
        request.set_sender_id(sender_id);
        request.set_receiver_id(receiver_id);
        request.set_chat_msg(chat_content);

        send_chat_msg_res reply;

        ClientContext context;
        Status status = m_stub->send_chat_msg(&context, request, &reply);

        if (status.ok()) {
            if (reply.result() != 0) {
                log_error("Failed send message to im client!");
                return -1;
            }
            return 0;
        } else {
            log_error("Call to im client failed! err msg: %s", status.error_message().c_str());
            return -1;
        }
    }

private:
    std::unique_ptr<prod_im_client_service::Stub> m_stub;
};


int
prod_im_s_mod_chat_msg_relay::relay_msg(const std::string& peer_ip,
                                        uint16_t peer_port,
                                        const std::string& sender_id,
                                        const std::string& receiver_id,
                                        const std::string& chat_content)
{
    std::string im_client_listen_addr = peer_ip + ":" + std::to_string(peer_port);
    CallImClient call_to_im_client(grpc::CreateChannel(im_client_listen_addr,
                                                       grpc::InsecureChannelCredentials()));
    return call_to_im_client.SendMsg(sender_id, receiver_id, chat_content);
}
