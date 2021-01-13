#ifndef CPP_TOOLKIT_MOD_TIMEOUT_TABLE_H
#define CPP_TOOLKIT_MOD_TIMEOUT_TABLE_H

#ifdef __cplusplus
extern "C" {
#endif

#define cpt_timeout_table_t void*
#define cpt_timeout_table_val_t void*

typedef int (*cpt_timeout_table_free_val_func_t)(
        cpt_timeout_table_val_t val, void* ctx);

typedef struct {
    size_t timeout_steps;
    cpt_timeout_table_free_val_func_t free_val_func;
    void* user_ctx;
} cpt_timeout_table_param_t;

cpt_timeout_table_t cpt_timeout_table_create();
int cpt_timeout_table_destroy(cpt_timeout_table_t timeout_table);

int cpt_timeout_table_add(cpt_timeout_table_val_t val);
int cpt_timeout_table_del(cpt_timeout_table_val_t val);

int cpt_timeout_table_step();

#ifdef __cplusplus
}
#endif

#endif //CPP_TOOLKIT_MOD_TIMEOUT_TABLE_H
