#ifndef CPP_TOOLKIT_MOD_NP_QUEUE_HPP
#define CPP_TOOLKIT_MOD_NP_QUEUE_HPP

#include <functional>
#include <mod_atomic_queue/atomic_queue.h>

using np_queue_aq_t = atomic_queue::AtomicQueue2<void*, 65535>;

// Notify to polling queue
class np_queue_t {
public:
    np_queue_t(std::function<void()>&& notify_handler,
               std::function<void(void*)> dequeue_handler)
               : m_notify_handler(notify_handler),
                 m_dequeue_handler(dequeue_handler)
               {}
    int enqueue(void* p);
    int dequeue();
    int dequeue_wait();
    int dequeue_co();

private:
    np_queue_aq_t m_queue;

    std::atomic<bool> m_is_notify_mode = true;
    std::atomic<bool> m_notified = false;

    std::function<void()> m_notify_handler;
    std::function<void(void*)> m_dequeue_handler;
};


#endif //CPP_TOOLKIT_MOD_NP_QUEUE_HPP
