#include <app_asio_socket/app_asio_socket.h>

#include <mod_common/log.h>

#include "app_asio_socket_client.h"
#include "app_asio_socket_server.h"

static void print_usage()
{
    log_error("Usage:");
    log_info("Eg: program c <host> <port>");
    log_info("Eg: program s <port>");
}


int app_asio_socket(int argc, char** argv)
{
    if (argc < 2) {
        log_error("wrong parameters.");
        print_usage();
        return -1;
    }

    if (0 == strcmp(argv[1], "c")) {
        return app_asio_socket_client(argc - 1, argv + 1);
    } else if (0 == strcmp(argv[1], "s")) {
        return app_asio_socket_server(argc - 1, argv + 1);
    } else {
        log_error("wrong parameters.");
        print_usage();
        return -1;
    }
}
