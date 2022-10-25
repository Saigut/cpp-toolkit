#include <app_prod_im/app_prod_im.h>

#include <stdio.h>
#include <mod_common/log.h>

#include "app_prod_im_internal.h"

static void print_usage()
{
    printf("Usage:\n");
//    printf("Eg: program c <host> <port>\n");
    printf("Eg: program s\n");
}

int app_prod_im(int argc, char** argv)
{
    if (argc < 2) {
        log_error("wrong parameters.");
        print_usage();
        return -1;
    }

    if (0 == strcmp(argv[1], "c")) {
        return -1;
    } else if (0 == strcmp(argv[1], "s")) {
        return app_prod_im_server(argc - 1, argv + 1);
    } else {
        log_error("wrong parameters.");
        print_usage();
        return -1;
    }
}
