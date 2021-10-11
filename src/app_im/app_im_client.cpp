#include "app_im_client.h"

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <thread>
#include <ctime>

#include <mod_queue/mod_queue.h>
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


void ClientSendMsg::do_work()
{
    std::string msg = "msg from: " + std::to_string(m_my_id);
    WorkUtils::Timer_Asio timer{m_io_worker, shared_from_this()};
    while (true) {
        if (!m_client->send_msg(shared_from_this(), m_peer_id, msg)) {
            log_error("failed to send message to server!");
            break;
        }
        timer.wait_for(5000);
    }
}

void ClientRecvMsg::do_work()
{
    while (true) {
        uint64_t id;
        std::string msg;
        if (!m_client->recv_msg(shared_from_this(), id, msg)) {
            log_error("failed to receive message from server!");
            break;
        }
        log_info("[msg:%llu] %s", id, msg.c_str());
    }
}

bool im_client_impl::connect(Worker_NetIo* worker,
                             std::shared_ptr<Work> consignor_work)
{
    im_channel_builder_impl channel_builder{worker};
    server_msg_channel = channel_builder.connect(consignor_work, m_id, m_server_addr, m_server_port);
    if (!server_msg_channel) {
        log_error("connect failed!");
        return false;
    } else {
        return true;
    }
}

bool im_client_impl::send_msg(std::shared_ptr<Work> consignor_work, uint64_t id, std::string& msg)
{
    return server_msg_channel->send_msg(consignor_work, id, msg);
}
bool im_client_impl::recv_msg(std::shared_ptr<Work> consignor_work, uint64_t& id, std::string& msg)
{
    return server_msg_channel->recv_msg(consignor_work, id, msg);
}

class ClientWork : public Work {
public:
    ClientWork(Worker* main_worker,
               Worker_NetIo* io_worker,
               std::shared_ptr<im_client_impl> client,
               uint64_t peer_id)
            : m_main_worker(main_worker),
              m_io_worker(io_worker),
              m_client(client),
              m_peer_id(peer_id) {}

    void do_work() override
    {
        if (m_client->connect(m_io_worker, shared_from_this())) {
            m_main_worker->add_work(
                    new WorkWrap(std::make_shared<ClientRecvMsg>(m_client),
                                 nullptr));
            m_main_worker->add_work(
                    new WorkWrap(std::make_shared<ClientSendMsg>(m_client,
                                                                 m_client->m_id,
                                                                 m_peer_id,
                                                                 m_io_worker),
                                 nullptr));
        } else {
            log_error("client failed to connect!");
        }
    };

private:
    Worker* m_main_worker;
    Worker_NetIo* m_io_worker;
    std::shared_ptr<im_client_impl> m_client;
    uint64_t m_peer_id;
};

static void main_worker_thread(Worker* worker)
{
    worker->run();
}

static void worker_thread(Worker_NetIo* worker)
{
    worker->run();
}

// program <server_ip> <my_id> <peer_id>
int app_im_client_new(int argc, char** argv)
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

    Worker worker{};
    Worker_NetIo worker_net_io{};
    // Start main worker
    std::thread main_worker_thr(main_worker_thread, &worker);
    // Start net io worker
    std::thread other_worker_thr(worker_thread, &worker_net_io);

    std::string server_addr = server_ip;
    auto client = std::make_shared<im_client_impl>((uint64_t)my_id, server_addr, 12345);

    worker_net_io.wait_worker_started();
    worker.add_work(new WorkWrap(std::make_shared<ClientWork>(&worker,
                                                              &worker_net_io,
                                                              client,
                                                              (uint64_t)peer_id),
                                 nullptr));

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
