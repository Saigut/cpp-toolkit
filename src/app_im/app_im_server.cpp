#include "app_im_server.h"

#include <inttypes.h>
#include <mod_common/expect.h>

#include <grpcpp/grpcpp.h>
#include <cpt_im.grpc.pb.h>
#include <grpcpp/ext/proto_server_reflection_plugin.h>
#include <mod_worker/mod_worker.h>

#include "app_im_client.h"

using grpc::Channel;
using grpc::Status;
using grpc::ClientContext;
using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;


void RecvMsgFromClient::do_work() {
    while (true) {
        uint64_t id;
        std::string msg;
        if (!m_client_channel->recv_msg(shared_from_this(), id, msg)) {
            log_error("client channel failed to receive message!");
            m_id_channel_map.erase(m_client_id);
            break;
        }
        auto result = m_id_channel_map.find(id);
        if (result != m_id_channel_map.end()) {
            result->second.msg_q->push(msg);
        } else {
            log_warn("No channel for client id: %llu!", id);
        }
    }
}

void RelayMsgToClient::do_work() {
    WorkUtils::Timer_Asio timer{m_io_worker, shared_from_this()};
    while (true) {
        // Get message from relay queue
        if (m_msg_q->empty()) {
            timer.wait_for(1);
        } else {
            std::string& msg = m_msg_q->front();

            if (!m_client_channel->send_msg(shared_from_this(), m_client_id, msg)) {
                log_error("client channel failed to send message!");
                break;
            }
            m_msg_q->pop();
        }
    }
}

bool im_server_impl::listen() {
    return m_channel_builder.listen(m_server_addr, m_server_port);
}

bool im_server_impl::accept_channel(std::shared_ptr<Work> consignor_work) {
    uint64_t peer_id;
    std::shared_ptr<im_channel_impl> client_channel = m_channel_builder.accept(consignor_work, peer_id);
    if (!client_channel) {
        log_error("accept failed!");
        return false;
    } else {
        log_info("Accepted new client: %llu", peer_id);
        auto ret = m_id_channel_map.emplace(peer_id, client_channel);
        if (!ret.second) {
            log_error("failed insert to map!");
            return false;
        }
        m_main_worker->add_work(new WorkWrap(std::make_shared<RelayMsgToClient>(peer_id,
                                                                                client_channel,
                                                                                ret.first->second.msg_q,
                                                                                m_io_worker),
                                             nullptr));
        m_main_worker->add_work(new WorkWrap(std::make_shared<RecvMsgFromClient>(peer_id,
                                                                                 client_channel,
                                                                                 m_id_channel_map),
                                             nullptr));
        return true;
    }
}

void im2_light_channel_server_work::do_work() {
    while (true) {
        std::string msg;
        if (!m_channel->recv_msg(shared_from_this(), msg)) {
            log_error("channel failed to receive message!");
            break;
        }

        uint64_t id_in_msg;
        std::string chat_msg;
        if (m_channel->get_chat_msg(msg, id_in_msg, chat_msg)) {
            log_info("got message: %s", chat_msg.c_str());
            if (!m_channel->m_is_initiator) {
                std::string res_msg = "response from " + std::to_string(m_my_id);
                m_channel->send_text(shared_from_this(), 1234, res_msg);
            }
        } else {
            log_info("not chat message?");
        }
    }
}

void im2_server_work::do_work()
{
    std::string addr = "0.0.0.0";
    im2_channel_builder channel_builder{m_main_worker_sp, m_io_worker};

    expect_ret(channel_builder.listen(addr, 12345));
    while (true) {
        uint64_t peer_id;
        auto channel = channel_builder.accept(shared_from_this(), peer_id);
        if (!channel) {
            log_error("channel failed to accept!");
            break;
        }
        m_main_worker_sp->add_work(new WorkWrap(std::make_shared<im2_channel_recv_work>(channel)));
    }
}

class ServerWork : public Work {
public:
    ServerWork(Worker* main_worker,
               Worker_NetIo* io_worker,
               im_server_impl& server)
            : m_main_worker(main_worker),
              m_io_worker(io_worker),
              m_server(server) {}

    void do_work() override
    {
        while (m_server.accept_channel(shared_from_this()))
        {}
    };

private:
    Worker* m_main_worker;
    Worker_NetIo* m_io_worker;
    im_server_impl& m_server;
};

static void main_worker_thread(Worker* worker)
{
    worker->run();
}

static void worker_thread(Worker_NetIo* worker)
{
    worker->run();
}

int app_im_server_new(int argc, char** argv)
{
    Worker worker{};
    Worker_NetIo worker_net_io{};
    // Start main worker
    std::thread main_worker_thr(main_worker_thread, &worker);
    // Start net io worker
    std::thread other_worker_thr(worker_thread, &worker_net_io);


    std::string addr = "0.0.0.0";
    im_server_impl server{addr, 12345, &worker, &worker_net_io};

    expect_ret_val(server.listen(), -1);

    worker_net_io.wait_worker_started();
    worker.add_work(new WorkWrap(std::make_shared<ServerWork>(&worker, &worker_net_io, server), nullptr));

    while (true)  {
        std::this_thread::sleep_for(std::chrono::milliseconds (100));
    }

    return 0;
}

int app_im2_server(int argc, char** argv)
{
    auto worker = std::make_shared<Worker>();
    auto worker_net_io = std::make_shared<Worker_NetIo>();
    // Start main worker
    std::thread main_worker_thr(main_worker_thread, &(*worker));
    // Start net io worker
    std::thread other_worker_thr(worker_thread, &(*worker_net_io));

    std::string addr = "0.0.0.0";

    worker_net_io->wait_worker_started();
    worker->add_work(new WorkWrap(std::make_shared<im2_server_work>(
            addr, 12345,
            worker,
            worker_net_io,
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
