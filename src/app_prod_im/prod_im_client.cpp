#include "prod_im_client.hpp"

#include <time.h>
#include <thread>
#include <grpcpp/grpcpp.h>
#include <grpcpp/ext/proto_server_reflection_plugin.h>
#include <mod_common/log.hpp>
#include <mod_common/expect.hpp>
#include <mod_common/utils.hpp>

#include "prod_im_client_mod_main.hpp"
#include "prod_im_client_mod_cli.hpp"


using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;


std::shared_ptr<prod_im_c_mod_main> g_client_main;

static void run_read_loop()
{
    prod_im_client_mod_cli_read_loop();
}

static void run_get_chat_msg()
{

    while (true) {
        auto msg_list = g_client_main->get_chat_msg();
        if (!msg_list || msg_list->empty()) {
            cppt_msleep(200);
        } else {
            for (auto& msg : *msg_list) {
                g_client_main->process_chat_msg(msg.sender_id,
                                                msg.chat_msg);
            }
        }
    }
}

// argc >= 2
int app_prod_im_client(int argc, char** argv)
{
    srand(time(NULL));

    std::string server_ip = "127.0.0.1";
    uint16_t server_port = 60100;
    std::string my_id = argv[1];
    std::string im_server_listen_addr = server_ip + ":" + std::to_string(server_port);
    auto im_server_grpc_api = std::make_shared<call_im_server_api_grpc>(
            grpc::CreateChannel(im_server_listen_addr,
                                grpc::InsecureChannelCredentials()));
    g_client_main = std::make_shared<prod_im_c_mod_main>(
            server_ip,
            server_port,
            0,
            my_id,
            im_server_grpc_api);

    printf("Client info:\n");
    printf("client user id: %s\n", my_id.c_str());
    printf("server ip: %s\n", server_ip.c_str());
    printf("server port: %u\n", server_port);

    std::thread thr_read_loop{ run_read_loop };
    cppt_msleep(500);
    std::thread thr_get_chat_msg{ run_get_chat_msg };

    thr_get_chat_msg.join();
    thr_read_loop.join();

    return 0;
}
