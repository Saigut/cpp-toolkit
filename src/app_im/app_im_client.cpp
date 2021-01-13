#include "app_im_client.h"


app_im_client_t app_im_client_create(app_im_client_param_t* param)
{
    // create send to server queue
    // create receiving from server queue
    return 0;
}

int app_im_client_destroy(app_im_client_t app_im_client)
{
    return 0;
}

int app_im_client_start(app_im_client_t app_im_client)
{
    while (true) {
        // read receiving queue
        // deal with msg
    }
    return 0;
}

int app_im_client_send_msg(app_im_client_msg_t* msg)
{
    // write msg to server queue
    return 0;
}

