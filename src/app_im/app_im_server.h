#ifndef CPP_TOOLKIT_APP_IM_SERVER_H
#define CPP_TOOLKIT_APP_IM_SERVER_H

#ifdef __cplusplus
extern "C" {
#endif

#define app_im_server_t void*

typedef struct {

} app_im_server_param_t;

app_im_server_t app_im_server_create(app_im_server_param_t* param);
int app_im_server_destroy(app_im_server_t app_im_server);

int app_im_server_start(app_im_server_t app_im_server);

#ifdef __cplusplus
}
#endif

#endif //CPP_TOOLKIT_APP_IM_SERVER_H
