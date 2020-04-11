#include <app_chat/app_chat.h>

#include <stdio.h>
#include <mod_common/log.h>

#include "app_chat_client.h"
#include "app_chat_server.h"

static void print_usage()
{
    log_error("Usage:");
    log_info("Eg: program c <host> <port>");
    log_info("Eg: program s <port> [<port> ...]");
}

int app_chat(int argc, char** argv)
{
    if (argc < 2) {
        log_error("wrong parameters.");
        print_usage();
        return -1;
    }

    if (0 == strcmp(argv[1], "c")) {
        return app_chat_client(argc - 1, argv + 1);
    } else if (0 == strcmp(argv[1], "s")) {
        return app_chat_server(argc - 1, argv + 1);
    } else {
        log_error("wrong parameters.");
        print_usage();
        return -1;
    }
}
