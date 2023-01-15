#include "app_im_client.hpp"

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <thread>
#include <ctime>

#include <mod_queue/mod_queue.hpp>
#include <mod_common/expect.hpp>

#include <grpcpp/grpcpp.h>
#include <cpt_im.grpc.pb.h>
#include <grpcpp/ext/proto_server_reflection_plugin.h>

using grpc::Channel;
using grpc::Status;
using grpc::ClientContext;
using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;


static io_context gs_io_ctx;

static void main_worker_thread(Worker* worker)
{
    worker->run();
}

static void worker_thread(io_context* io_ctx)
{
    boost::asio::io_context::work io_work(*io_ctx);
    io_ctx->run();
}

void im2_client_request_work::do_work() {
    std::shared_ptr<Work> this_obj = shared_from_this();
    WorkUtils::WorkCoCbs co_cbs{[this_obj]() { this_obj->m_wp.wp_yield(0); },
                                [this_obj]() { this_obj->add_self_back_to_main_worker(nullptr); }};
    auto channel = std::make_shared<im2_channel>(*m_channel);
    channel->m_tcp.m_co_cbs = co_cbs;
    auto light_channel = std::make_shared<im2_light_channel>(
            channel,
            channel->generate_light_ch_id(),
            true,
            co_cbs);
    std::string req = "msg from: " + std::to_string(m_my_id);
    WorkUtils::Timer_Asio timer{gs_io_ctx, co_cbs};
    while (true) {
        if (!light_channel->send_text(11111, req)) {
            log_error("failed to send request to server!");
            break;
        }
        uint64_t id_in_msg;
        std::string res;
        if (light_channel->recv_text(id_in_msg, res)) {
            log_info("got response: %s", res.c_str());
        }
        timer.wait_for(5000);
    }
}

void im2_client_work::do_work() {
    std::string addr = "127.0.0.1";
    uint16_t port = 12345;
    std::shared_ptr<Work> this_obj = shared_from_this();
    WorkUtils::WorkCoCbs co_cbs{[this_obj]() { this_obj->m_wp.wp_yield(0); },
                                [this_obj]() { this_obj->add_self_back_to_main_worker(nullptr); }};
    im2_channel_builder channel_builder{m_io_ctx, co_cbs};
    auto channel = channel_builder.connect(m_my_id, addr, port);
    if (!channel) {
        log_error("failed connect to %s:%u!", addr.c_str(), port);
    }
    m_main_worker_sp->add_work(new WorkWrap(std::make_shared<im2_client_request_work>(channel, m_my_id)));
    m_main_worker_sp->add_work(new WorkWrap(std::make_shared<im2_channel_recv_work>(channel)));
}

// program <server_ip> <my_id> <peer_id>
int app_im2_client(int argc, char** argv)
{
    int my_id;
    int peer_id;
    const char* server_ip;
    if (4 != argc) {
        log_error("Invalid arguments!\n");
        return -1;
    } else {
        server_ip = argv[1];
        my_id = atoi(argv[2]);
        expect_ret_val(my_id >= 0, -1);
        peer_id = atoi(argv[3]);
        expect_ret_val(peer_id >= 0, -1);
    }

    auto worker = std::make_shared<Worker>();
    // Start main worker
    std::thread main_worker_thr(main_worker_thread, &(*worker));

    std::thread other_worker_thr(worker_thread, &gs_io_ctx);

    std::string server_addr = server_ip;

    worker->add_work(new WorkWrap(std::make_shared<im2_client_work>(
            server_addr,
            12345,
            worker,
            gs_io_ctx,
            (uint64_t) my_id,
            (uint64_t) peer_id)));

    while (true)  {
        std::this_thread::sleep_for(std::chrono::milliseconds (100));
    }

    return 0;
}

// program <list_port> <user_id> <peer_user_id>
int app_im_client(int argc, char** argv)
{
    std::string port = argv[1];
    im_client client(port,
                     std::atoi(argv[2]),
                     std::atoi(argv[3]));
    expect_ret_val(0 == client.init(), -1);
    client.start();
    return 0;
}

int im_client::init()
{
//    this->r_queue = std::make_shared<cpt_queue<::cpt_im::ClientIntfReq>>();
//    this->s_queue = std::make_shared<cpt_queue<::cpt_im::ServerIntfReq>>();
    expect_ret_val(this->r_queue.init(), -1);
    expect_ret_val(this->s_queue.init(), -1);
    return 0;
}

class CptImClientServiceImpl : public ::cpt_im::CptImClient::Service {
public:
    explicit CptImClientServiceImpl(im_client* client): m_client(client) {}
    ::grpc::Status ClientIntf(
            ::grpc::ServerContext* context,
            const ::cpt_im::ClientIntfReq* request,
            ::cpt_im::ClientIntfRes* response) override {
        ::cpt_im::ClientIntfReq req;
        m_client->r_queue.write(*request);
        response->set_err_code(::cpt_im::emErrCode_Ok);
        return Status::OK;
    }
private:
    im_client* m_client;
};

void im_client::grpc_receive()
{
    std::string server_address("0.0.0.0:" + this->m_list_port);
    CptImClientServiceImpl service(this);

    grpc::EnableDefaultHealthCheckService(true);
    grpc::reflection::InitProtoReflectionServerBuilderPlugin();
    ServerBuilder builder;
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);
    std::unique_ptr<Server> server(builder.BuildAndStart());
    std::cout << "Client listening on " << server_address << std::endl;

    server->Wait();
}

void im_client::grpc_send()
{
    while (true) {
        ::cpt_im::ServerIntfReq req;
        if (s_queue.read(req)) {
            std::string target_str = "localhost:60000";
            std::unique_ptr<::cpt_im::CptImServer::Stub> stub;
            stub = ::cpt_im::CptImServer::NewStub(grpc::CreateChannel(
                    target_str, grpc::InsecureChannelCredentials()));

            ClientContext context;
            context.set_deadline(std::chrono::system_clock::now()
                + std::chrono::seconds(3));
            ::cpt_im::ServerIntfRes res;
            Status status = stub->ServerIntf(&context, req, &res);

        } else {
            std::this_thread::sleep_for (
                    std::chrono::milliseconds(5));
        }
    }
}

void im_client::send_msg_thread() {
    std::string input_msg;
    while (true) {
        printf("Input msg > ");
        std::getline (std::cin, input_msg);
        if (!input_msg.empty()) {
            send_msg(input_msg);
        }
        std::this_thread::sleep_for (
                std::chrono::milliseconds(5));
    }
}

void im_client::start() {

    std::time_t last_hb_time = std::time(nullptr);
    std::time_t cur_time;
    std::string input_msg;

    std::thread thr_grpc_send(&im_client::grpc_send, this);
    std::thread thr(&im_client::send_msg_thread, this);
    std::thread thr_grpc_recv(&im_client::grpc_receive, this);

    while (true) {
        ::cpt_im::ClientIntfReq req;
        send_hb();
        if (this->r_queue.read(req)) {
            expect(0 == deal_with_msg(req));
        } else {
            std::this_thread::sleep_for (
                    std::chrono::milliseconds(5));
        }
        cur_time = std::time(nullptr);
        if (cur_time > last_hb_time + 5) {
            send_hb();
            last_hb_time = cur_time;
        }
    }
}

int im_client::send_msg(const std::string& msg) {
    ::cpt_im::ServerIntfReq req;
    req.set_msg_type(::cpt_im::emServerMsgType_Chat);
    req.set_client_listen_port(this->m_list_port);
    req.set_client_user_id(this->m_user_id);
    ::cpt_im::ChatMsg* chat_msg = req.mutable_chat_msg();
    chat_msg->set_src_user_id(this->m_user_id);
    chat_msg->set_dst_user_id(this->m_peer_user_id);
    chat_msg->set_msg_text(msg);
    if (this->s_queue.write(req)) {
        return 0;
    } else {
        return -1;
    }
}

int im_client::deal_with_msg(const ::cpt_im::ClientIntfReq& req) {
    switch (req.msg_type()) {
        case ::cpt_im::emClientMsgType_Chat: {
            printf("\nFrom %" PRIu64 " to %" PRIu64 ", msg > %s\n",
                     req.chat_msg().src_user_id(),
                     req.chat_msg().dst_user_id(),
                     req.chat_msg().msg_text().c_str());
            break;
        }
        default:
            log_error("Wrong msg type!");
            return -1;
    }
    return 0;
}

int im_client::send_hb() {
    ::cpt_im::ServerIntfReq req;
    req.set_msg_type(::cpt_im::emServerMsgType_Control);
    req.set_client_listen_port(this->m_list_port);
    req.set_client_user_id(this->m_user_id);
    ::cpt_im::ControlMsg* control_msg = req.mutable_control_msg();
    control_msg->set_user_id(this->m_user_id);
    control_msg->set_control_type(::cpt_im::emControlMsgType_HeartBeat);
    if (0 == this->s_queue.write(req)) {
        return 0;
    } else {
        return -1;
    }
}
