#include "app_im_server.hpp"

#include <inttypes.h>
#include <mod_common/expect.hpp>

#include <grpcpp/grpcpp.h>
#include <cpt_im.grpc.pb.h>
#include <grpcpp/ext/proto_server_reflection_plugin.h>
#include <mod_worker/mod_worker.hpp>

#include "app_im_client.hpp"

using grpc::Channel;
using grpc::Status;
using grpc::ClientContext;
using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;


void im2_light_channel_server_work::do_work() {
    std::shared_ptr<Work> this_obj = shared_from_this();
    WorkUtils::WorkCoCbs co_cbs{[this_obj]() { this_obj->m_wp.wp_yield(0); },
                                [this_obj]() { this_obj->add_self_back_to_main_worker(nullptr); }};
    auto main_channel = std::make_shared<im2_channel>(m_main_channel);
    main_channel->m_tcp.m_co_cbs = co_cbs;
    auto light_channel = *m_channel;
    light_channel.m_main_channel = main_channel;
    light_channel.m_co_cbs = co_cbs;
    while (true) {
        std::string req;
        uint64_t id_in_msg;
        if (!light_channel.recv_text(id_in_msg, req)) {
            log_error("failed to receive request!");
            break;
        }
        log_info("got request: %s", req.c_str());
        std::string res = "response from " + std::to_string(m_my_id);
        light_channel.send_text(1234, res);
    }
}

void im2_server_work::do_work()
{
    std::string addr = "0.0.0.0";
    std::shared_ptr<Work> this_obj = shared_from_this();
    WorkUtils::WorkCoCbs co_cbs{[this_obj]() { this_obj->m_wp.wp_yield(0); },
                                [this_obj]() { this_obj->add_self_back_to_main_worker(nullptr); }};
    im2_channel_builder channel_builder{m_io_ctx, co_cbs};

    expect_ret(channel_builder.listen(addr, 12345));
    while (true) {
        uint64_t peer_id;
        auto channel = channel_builder.accept(peer_id);
        if (!channel) {
            log_error("channel failed to accept!");
            break;
        }
        m_main_worker_sp->add_work(new WorkWrap(std::make_shared<im2_channel_recv_work>(channel)));
    }
}


static void main_worker_thread(Worker* worker)
{
    worker->run();
}

static void worker_thread(io_context* io_ctx)
{
    boost::asio::io_context::work io_work(*io_ctx);
    io_ctx->run();
}

int app_im2_server(int argc, char** argv)
{
    auto worker = std::make_shared<Worker>();
    auto worker_net_io = std::make_shared<Worker_NetIo>();
    // Start main worker
    std::thread main_worker_thr(main_worker_thread, &(*worker));

    io_context io_ctx;
    std::thread other_worker_thr(worker_thread, &io_ctx);

    std::string addr = "0.0.0.0";

    worker_net_io->wait_worker_started();
    worker->add_work(new WorkWrap(std::make_shared<im2_server_work>(
            addr, 12345,
            worker,
            io_ctx,
            22222)));

    while (true)  {
        std::this_thread::sleep_for(std::chrono::milliseconds (100));
    }

    return 0;
}

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
                log_error("Don't know listen port of user: %" PRIu64,
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
            printf("From %" PRIu64 " to %" PRIu64 ", msg > %s\n",
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
