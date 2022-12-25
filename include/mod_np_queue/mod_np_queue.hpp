#ifndef CPP_TOOLKIT_MOD_NP_QUEUE_HPP
#define CPP_TOOLKIT_MOD_NP_QUEUE_HPP

#include <functional>
#include <mod_atomic_queue/atomic_queue.h>


// Notify to polling queue。多写单读
template <class eleT>
class np_queue_t {
private:
    using np_queue_aq_t = atomic_queue::AtomicQueue2<eleT, 65535>;
public:
    np_queue_t() = default;;
    np_queue_t(std::function<bool(eleT&)>&& dequeue_handler,
               std::function<void()>&& notify_handler,
               std::function<bool()>&& waiting_handler)
               : m_notify_handler(notify_handler),
                 m_dequeue_handler(dequeue_handler),
                 m_waiting_handler(waiting_handler) {}
    void set_handlers(std::function<bool(eleT&)> &&dequeue_handler,
                      std::function<void()> &&notify_handler,
                      std::function<bool()> &&waiting_handler);
    bool try_enqueue(eleT&& p);
    bool enqueue(eleT&& p);
    bool do_dequeue_wait();

private:
    bool do_dequeue();
    int notify();

    np_queue_aq_t m_queue;

    std::atomic<bool> m_is_notify_mode = true;
    std::atomic<bool> m_notified = false;

    std::function<void()> m_notify_handler;

    // 从队列读出的元素会交由 m_dequeue_handler 处理
    // ret: true, 继续读队列; false, 提前退出
    std::function<bool(eleT&)> m_dequeue_handler;

    // 读到队列为空时即调用 m_waiting_handler。不可为 null
    // ret: true, 继续 dequeue; false, 不 dequeue 了。
    std::function<bool()> m_waiting_handler;
};

template <class eleT>
void np_queue_t<eleT>::set_handlers(
        std::function<bool(eleT&)> &&dequeue_handler,
        std::function<void()> &&notify_handler,
        std::function<bool()> &&waiting_handler)
{
    m_dequeue_handler = dequeue_handler;
    m_notify_handler = notify_handler;
    m_waiting_handler = waiting_handler;
}

template <class eleT>
int np_queue_t<eleT>::notify()
{
    // 检查模式。notify 模式则通知
    if (m_is_notify_mode) {
        if (!m_notified) {
            // notify
            if (m_notify_handler) {
                // notify handler 需要是线程安全的
                m_notify_handler();
            } else {
                return -1;
            }
            m_notified = true;
        }
    }
    return 0;
}

template <class eleT>
bool np_queue_t<eleT>::enqueue(eleT&& p)
{
    // 1. 写入
    m_queue.push(p);
    // 2. 通知
    if (0 != notify()) {
        return true;
    }
    return true;
}

template <class eleT>
bool np_queue_t<eleT>::try_enqueue(eleT&& p)
{
    // 1. 写入
    if (!m_queue.try_push(p)) {
        return false;
    }
    // 2. 检查模式。notify 模式则通知。
    if (0 != notify()) {
        return true;
    }
    return true;
}

template <class eleT>
bool np_queue_t<eleT>::do_dequeue()
{
    eleT p;
    // 1. 切换为 poll 模式
    m_is_notify_mode = false;
    // 2. poll
    while (m_queue.try_pop(p)) {
        // call handler
        if (m_dequeue_handler) {
            if (!m_dequeue_handler(p)) {
                return false;
            }
        }
    }
    // 3. 切换为 notify 模式
    m_notified = false;
    m_is_notify_mode = true;
    while (!m_notified && m_queue.try_pop(p)) {
        // call handler
        if (m_dequeue_handler) {
            if (!m_dequeue_handler(p)) {
                return false;
            }
        }
    }
    return true;
}

template <class eleT>
bool np_queue_t<eleT>::do_dequeue_wait()
{
    if (!m_waiting_handler) {
        return false;
    }

    do {
        if (!do_dequeue()) {
            return true;
        }
    } while (m_waiting_handler());

    return true;
}

#endif //CPP_TOOLKIT_MOD_NP_QUEUE_HPP
