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
    np_queue_t(std::function<void()>&& notify_handler,
               std::function<bool()>&& waiting_handler)
               : m_notify_handler(notify_handler),
                 m_waiting_handler(waiting_handler) {}
    void set_handlers(std::function<void()>&& notify_handler,
                      std::function<bool()>&& waiting_handler);
    bool try_enqueue(eleT&& p);
    bool enqueue(eleT&& p);
    bool dequeue(eleT& p);

private:
    int notify();

    np_queue_aq_t m_queue;

    std::atomic<bool> m_is_notify_mode = true;
    std::atomic<bool> m_notified = false;

    std::function<void()> m_notify_handler;

    // 读到队列为空时即调用 m_waiting_handler。不可为 null
    // ret: true, 继续 dequeue; false, 不 dequeue 了。
    std::function<bool()> m_waiting_handler;
};

template <class eleT>
void np_queue_t<eleT>::set_handlers(
        std::function<void()> &&notify_handler,
        std::function<bool()> &&waiting_handler)
{
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

template<class eleT>
bool np_queue_t<eleT>::dequeue(eleT& p)
{
    if (m_queue.try_pop(p)) {
        return true;
    }

    m_is_notify_mode = true;
    do {
        m_notified = false;
        if (m_queue.try_pop(p)) {
            m_is_notify_mode = false;
            return true;
        }
        if (!m_waiting_handler) {
            m_is_notify_mode = false;
            return false;
        }
    } while(m_waiting_handler());

    m_is_notify_mode = false;
    return false;
}

#endif //CPP_TOOLKIT_MOD_NP_QUEUE_HPP
