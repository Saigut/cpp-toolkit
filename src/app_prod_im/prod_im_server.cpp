#include "app_prod_im_internal.h"

#include <thread>
#include <grpcpp/grpcpp.h>
#include <grpcpp/ext/proto_server_reflection_plugin.h>
#include <mod_common/log.h>


using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;

prod_im_s_mod_main g_server_main;

static void run_main()
{
    g_server_main.run();
}

static void run_grpc_api()
{
    std::string server_address("0.0.0.0:60100");
    prod_im_server_grpc_api_impl service;

    grpc::EnableDefaultHealthCheckService(true);
    grpc::reflection::InitProtoReflectionServerBuilderPlugin();
    ServerBuilder builder;

    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);

    std::unique_ptr<Server> server(builder.BuildAndStart());
    log_info("Prod im server listening on: %s", server_address.c_str());

    server->Wait();
}

int app_prod_im_server(int argc, char** argv)
{
    std::thread thr_main{run_main};
    std::thread thr_grpc_api{run_grpc_api};

    thr_grpc_api.join();
    thr_main.join();

    return 0;
}
