#include "app_prod_im_internal.h"

#include <time.h>
#include <thread>
#include <grpcpp/grpcpp.h>
#include <grpcpp/ext/proto_server_reflection_plugin.h>
#include <mod_common/log.h>
#include <mod_common/expect.h>
#include <mod_common/utils.h>

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;


std::shared_ptr<prod_im_c_mod_main> g_client_main;


static void run_grpc_api(uint16_t listen_port)
{
    std::string listen_address = "0.0.0.0:" + std::to_string(listen_port);

    prod_im_client_grpc_api_impl service;

    grpc::EnableDefaultHealthCheckService(true);
    grpc::reflection::InitProtoReflectionServerBuilderPlugin();
    ServerBuilder builder;

    builder.AddListeningPort(listen_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);

    std::unique_ptr<Server> grpc_server(builder.BuildAndStart());
    log_info("Prod im client listening on: %s", listen_address.c_str());

    grpc_server->Wait();
}


int app_prod_im_client(int argc, char** argv)
{
    if (argc < 3) {
        printf("Invalid number of parameters!\n");
        print_client_cli_usage();
        return -1;
    }

    srand(time(NULL));

    std::string server_ip = "127.0.0.1";
    uint16_t server_port = 60100;
    uint16_t client_port = 10000 + (rand() % 1000);

    std::string my_id = argv[1];

    std::string im_server_listen_addr = server_ip + ":" + std::to_string(server_port);
    auto im_server_grpc_api = std::make_shared<call_im_server_grpc>(
            grpc::CreateChannel(im_server_listen_addr,
                                grpc::InsecureChannelCredentials()));

    g_client_main = std::make_shared<prod_im_c_mod_main>(
            server_ip,
            server_port,
            client_port,
            my_id,
            im_server_grpc_api);


    printf("Client info:\n");
    printf("client user id: %s\n", my_id.c_str());
    printf("client port: %u\n", client_port);
    printf("server ip: %s\n", server_ip.c_str());
    printf("server port: %u\n", server_port);

//    cppt_msleep(150);
    prod_im_client_mod_cli(argc - 2, argv + 2);

    if (0 == strcmp("login", *(argv + 2))) {
        std::thread thr_grpc_api{run_grpc_api, client_port};
        thr_grpc_api.join();
    }

    return 0;
}
