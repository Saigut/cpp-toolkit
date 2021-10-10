#include "app_im/app_im.h"

#include <mod_common/log.h>

#include "app_im_client.h"
#include "app_im_server.h"

int app_im(int argc, char** argv)
{
    if (argc < 2) {
        log_error("wrong parameters.");
//        print_usage();
        return -1;
    }

    if (0 == strcmp(argv[1], "c")) {
        return app_im_client_new(argc - 1, argv + 1);
    } else if (0 == strcmp(argv[1], "s")) {
        return app_im_server_new(argc - 1, argv + 1);
    } else {
        log_error("wrong parameters.");
//        print_usage();
        return -1;
    }
}
