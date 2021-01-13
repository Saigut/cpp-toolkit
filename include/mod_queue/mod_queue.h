#ifndef CPP_TOOLKIT_MOD_QUEUE_H
#define CPP_TOOLKIT_MOD_QUEUE_H

#ifdef __cplusplus
extern "C" {
#endif

#define cpt_queue_t void*

typedef struct {
    size_t queue_sz;    // 0 no limit;
} cpt_queue_param_t;

cpt_queue_t cpt_queue_create(cpt_queue_param_t* param);
int cpt_queue_destroy(cpt_queue_t queue);

int cpt_queue_write(cpt_queue_t queue, void* val);
int cpt_queue_read(cpt_queue_t queue, void** val);


#ifdef __cplusplus
}
#endif

#endif //CPP_TOOLKIT_MOD_QUEUE_H
