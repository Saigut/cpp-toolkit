#include "app_im_server.h"

#include <mod_common/expect.h>

#include <grpcpp/grpcpp.h>
#include <cpt_im.grpc.pb.h>
#include <grpcpp/ext/proto_server_reflection_plugin.h>


using grpc::Channel;
using grpc::Status;
using grpc::ClientContext;
using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;

int app_im_server(int argc, char** argv)
{
    // create server
    // receiving and deal with message

    im_server server;

    expect_ret_val(0 == server.init(), -1);

    server.start();

    return 0;
}

int im_server::init() {

//    this->r_queue = cpt_queue<::cpt_im::ServerIntfReq>();
//    this->s_queue = std::make_shared<cpt_queue<::cpt_im::ClientIntfReq>>();

    expect_ret_val(this->r_queue.init(), -1);
    expect_ret_val(this->s_queue.init(), -1);
    expect_ret_val(this->uid_uport_table.init(), -1);

    return 0;
}

class CptImServerServiceImpl : public ::cpt_im::CptImServer::Service {
public:
    explicit CptImServerServiceImpl(im_server* server): m_server(server) {}
    ::grpc::Status ServerIntf(
            ::grpc::ServerContext* context,
            const ::cpt_im::ServerIntfReq* request,
            ::cpt_im::ServerIntfRes* response) override {
        m_server->r_queue.write(*request);
        response->set_err_code(::cpt_im::emErrCode_Ok);
        return Status::OK;
    }
private:
    im_server* m_server;
};

void im_server::grpc_receive()
{
    std::string server_address("0.0.0.0:60000");
    CptImServerServiceImpl service(this);

    grpc::EnableDefaultHealthCheckService(true);
    grpc::reflection::InitProtoReflectionServerBuilderPlugin();
    ServerBuilder builder;
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);
    std::unique_ptr<Server> server(builder.BuildAndStart());
    std::cout << "Server listening on " << server_address << std::endl;

    server->Wait();
}

void im_server::grpc_send()
{
    while (true) {
        ::cpt_im::ClientIntfReq req;
        if (this->s_queue.read(req)) {
            std::string client_port;
            if (this->uid_uport_table.find(
                    req.chat_msg().dst_user_id(),
                    client_port)) {
                std::string target_str = "localhost:" + client_port;
                std::unique_ptr<::cpt_im::CptImClient::Stub> stub;
                stub = ::cpt_im::CptImClient::NewStub(grpc::CreateChannel(
                        target_str, grpc::InsecureChannelCredentials()));
                ClientContext context;
                context.set_deadline(std::chrono::system_clock::now()
                + std::chrono::seconds(3));
                ::cpt_im::ClientIntfRes res;
                Status status = stub->ClientIntf(&context, req, &res);
            } else {
                log_error("Don't know listen port of user: %lu",
                          req.chat_msg().dst_user_id());
            }
        } else {
            std::this_thread::sleep_for (
                    std::chrono::milliseconds(5));
        }
    }
}

void im_server::start() {
    std::thread thr_grpc_send(&im_server::grpc_send, this);
    std::thread thr_grpc_recv(&im_server::grpc_receive, this);
    while (true) {
        ::cpt_im::ServerIntfReq req;
        if (this->r_queue.read(req)) {
            expect(0 == deal_with_msg(req));
        } else {
            std::this_thread::sleep_for (
                    std::chrono::milliseconds(5));
        }
    }
}

int im_server::deal_with_msg(const cpt_im::ServerIntfReq &req) {
    this->uid_uport_table.insert(
            req.client_user_id(),
            req.client_listen_port());
    switch (req.msg_type()) {
        case ::cpt_im::emServerMsgType_Chat: {
            printf("From %lu to %lu, msg > %s\n",
                     req.chat_msg().src_user_id(),
                     req.chat_msg().dst_user_id(),
                     req.chat_msg().msg_text().c_str());
            cpt_im::ClientIntfReq client_req;
            client_req.set_msg_type(
                    ::cpt_im::emClientMsgType_Chat);
            ::cpt_im::ChatMsg* chat_msg =
                    client_req.mutable_chat_msg();
            chat_msg->set_msg_id(req.chat_msg().msg_id());
            chat_msg->set_src_user_id(
                    req.chat_msg().src_user_id());
            chat_msg->set_dst_user_id(
                    req.chat_msg().dst_user_id());
            chat_msg->set_msg_type(
                    ::cpt_im::emChatMsgType_Text);
            chat_msg->set_msg_text(req.chat_msg().msg_text());
            expect(this->s_queue.write(client_req));
            break;
        }
        case ::cpt_im::emServerMsgType_Control: {
            const ::cpt_im::ControlMsg& msg = req.control_msg();
            if (::cpt_im::emControlMsgType_HeartBeat
                == msg.control_type()) {
//                update_user_session(msg.user_id());
            }
            break;
        }
        default:
            log_error("Wrong msg type!");
            return -1;
    }
    return 0;
}
