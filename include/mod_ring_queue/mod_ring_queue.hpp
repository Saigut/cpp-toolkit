#ifndef CPP_TOOLKIT_MOD_RING_QUEUE_HPP
#define CPP_TOOLKIT_MOD_RING_QUEUE_HPP

#include <memory>
#include <mod_mempool/mod_mempool.hpp>
#include <mod_atomic_queue/atomic_queue.hpp>


using ring_queue_aq = atomic_queue::AtomicQueue2<void*, 65535>;

class ring_queue {
public:
    explicit ring_queue(size_t capacity, mempool& _mempool)
    : m_capacity(capacity), m_mempool(_mempool) {}

    bool empty();
    bool full();
    void front(void*& p);
    int push(void* p);
    int pop(void*& p);
    mempool& get_mempool() {
        return m_mempool;
    }

private:
    size_t m_capacity;
    mempool& m_mempool;
    ring_queue_aq m_q;
    void* front_ele = nullptr;
};

class ring_queue_rw {
public:
    ring_queue_rw(std::shared_ptr<ring_queue> rq_r,
                  std::shared_ptr<ring_queue> rq_w)
    : m_rq_r(rq_r), m_rq_w(rq_w) {}
    std::shared_ptr<ring_queue> m_rq_r = nullptr;
    std::shared_ptr<ring_queue> m_rq_w = nullptr;
};

#endif //CPP_TOOLKIT_MOD_RING_QUEUE_HPP
