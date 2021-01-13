#ifndef CPP_TOOLKIT_MOD_HASH_TABLE_H
#define CPP_TOOLKIT_MOD_HASH_TABLE_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define cpt_htable_t void*
#define cpt_htable_key_t void*
#define cpt_htable_val_t void*

typedef uint64_t (*cpt_htable_hash_func_t)(
        cpt_htable_key_t key, void* ctx);
// ret, 0 equal, 1 key1 bigger, -1 key2 bigger
typedef int (*cpt_htable_cmp_func_t)(
        cpt_htable_key_t key1, cpt_htable_key_t key2, void* ctx);
typedef int (*cpt_htable_free_val_func_t)(
        cpt_htable_val_t val, void* ctx);

typedef struct {
    size_t key_sz;
    cpt_htable_hash_func_t hash_func;
    cpt_htable_cmp_func_t cmp_func;
    cpt_htable_free_val_func_t free_val_func;
    size_t timeout_ms;  // 0, not timeout
    void* user_ctx;
} cpt_htable_param_t;

cpt_htable_t cpt_htable_create(cpt_htable_param_t* param);
int cpt_htable_destroy(cpt_htable_t hash_table);

cpt_htable_val_t cpt_htable_find(cpt_htable_key_t key);
int cpt_htable_add(cpt_htable_key_t key, cpt_htable_val_t val);
int cpt_htable_del(cpt_htable_key_t key);

// for timeout
int cpt_htable_timeout_step(cpt_htable_t hash_table);

#ifdef __cplusplus
}
#endif

#endif //CPP_TOOLKIT_MOD_HASH_TABLE_H
