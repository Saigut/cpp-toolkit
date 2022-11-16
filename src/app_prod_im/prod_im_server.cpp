#include "prod_im_server.hpp"

#include <thread>
#include <mod_common/log.h>
#include <mod_common/utils.h>

#include "prod_im_server_mod_main.hpp"
#include "prod_im_server_mod_grpc_api.hpp"
#include "prod_im_server_mod_jsonrpc_api.hpp"


std::shared_ptr<prod_im_s_mod_main> g_server_main;

static void run_asio(boost::asio::io_context& io_ctx)
{
    boost::asio::io_context::work io_work(io_ctx);
    io_ctx.run();
    log_info("Asio thread quit.");
}

static void run_grpc_api()
{
    prod_im_server_grpc_api grpc_api;
    grpc_api.run();
}

static void run_jsonrpc_api()
{
    prod_im_server_jsonrpc_api jsonrpc_api;
    jsonrpc_api.run();
}

static void run_main()
{
    g_server_main->run();
}

int app_prod_im_server(int argc, char** argv)
{
    boost::asio::io_context io_ctx;

    g_server_main = std::make_shared<prod_im_s_mod_main>(io_ctx);

    std::thread thr_asio{ run_asio, std::ref(io_ctx) };
    std::thread thr_main{ run_main };
    cppt_msleep(200);
    std::thread thr_grpc_api{ run_grpc_api };
    std::thread thr_jsonrpc_api{ run_jsonrpc_api };

    thr_main.join();
    thr_grpc_api.join();
    thr_jsonrpc_api.join();
    thr_asio.join();

    return 0;
}
