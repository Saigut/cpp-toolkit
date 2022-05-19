#ifndef CPP_TOOLKIT_MOD_MEMPOOL_H
#define CPP_TOOLKIT_MOD_MEMPOOL_H

#include <stdio.h>
#include <stddef.h>
#include <queue>
#include <mod_atomic_queue/atomic_queue.h>


using mempool_aq = atomic_queue::AtomicQueue2<void*, 65535>;

class mempool {
public:
    explicit mempool(size_t ele_num, size_t ele_size)
    : m_ele_num(ele_num), m_ele_size(ele_size) {}
    int init();
    void deinit();
    void* alloc();
    void free(void* buf);
    void print_me() {
        printf("mempool: %p\n", this);
    }
private:
    size_t m_ele_num;
    size_t m_ele_size;
    void* m_buf = nullptr;
    mempool_aq m_q;
};

#endif //CPP_TOOLKIT_MOD_MEMPOOL_H
