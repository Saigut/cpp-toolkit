#ifndef CPP_TOOLKIT_APP_IM_CLIENT_H
#define CPP_TOOLKIT_APP_IM_CLIENT_H

#include <mod_queue/mod_queue.h>

#ifdef __cplusplus
extern "C" {
#endif

#define app_im_client_t void*

typedef struct {
} app_im_client_param_t;

app_im_client_t app_im_client_create(app_im_client_param_t* param);
int app_im_client_destroy(app_im_client_t app_im_client);

int app_im_client_start(app_im_client_t app_im_client);

typedef struct {
    char* msg_buf;
    size_t msg_len;
} app_im_client_msg_t;
int app_im_client_send_msg(app_im_client_msg_t* msg);

cpt_queue_t app_im_client_get_receiving_queue(app_im_client_t app_im_client);

int app_im_client(int argc, char** argv);

#ifdef __cplusplus
}
#endif

#endif //CPP_TOOLKIT_APP_IM_CLIENT_H
