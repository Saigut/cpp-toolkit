#ifndef CPP_TOOLKIT_APP_IM_CLIENT_H
#define CPP_TOOLKIT_APP_IM_CLIENT_H

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

} app_im_client_msg_t;
int app_im_client_send_msg(app_im_client_msg_t* msg);

#ifdef __cplusplus
}
#endif

#endif //CPP_TOOLKIT_APP_IM_CLIENT_H
