#ifndef CPP_TOOLKIT_MOD_NP_QUEUE_HPP
#define CPP_TOOLKIT_MOD_NP_QUEUE_HPP

#include <functional>
#include <mod_atomic_queue/atomic_queue.h>

using np_queue_aq_t = atomic_queue::AtomicQueue2<void*, 65535>;

// Notify to polling queue。多写单读
class np_queue_t {
public:
    np_queue_t(std::function<void()>&& notify_handler,
               std::function<bool(void*)>&& dequeue_handler,
               std::function<bool()>&& waiting_handler)
               : m_notify_handler(notify_handler),
                 m_dequeue_handler(dequeue_handler),
                 m_waiting_handler(waiting_handler) {}
    int enqueue(void* p);
    int do_dequeue_wait();

private:
    bool do_dequeue();

    np_queue_aq_t m_queue;

    std::atomic<bool> m_is_notify_mode = true;
    std::atomic<bool> m_notified = false;

    std::function<void()> m_notify_handler;

    // 从队列读出的元素会交由 m_dequeue_handler 处理
    // ret: true, 继续读队列; false, 提前退出
    std::function<bool(void*)> m_dequeue_handler;

    // 读到队列为空时即调用 m_waiting_handler。不可为 null
    // ret: true, 继续 dequeue; false, 不 dequeue 了。
    std::function<bool()> m_waiting_handler;
};


#endif //CPP_TOOLKIT_MOD_NP_QUEUE_HPP
