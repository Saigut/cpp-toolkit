#ifndef CPP_TOOLKIT_MOD_NP_QUEUE_HPP
#define CPP_TOOLKIT_MOD_NP_QUEUE_HPP

#include <functional>
#include <mod_atomic_queue/atomic_queue.h>


// Notify to polling queue。多写单读
template <class ELE_T, unsigned SIZE = 2048>
class np_queue_t {
private:
    using np_queue_aq_t = atomic_queue::AtomicQueue2<ELE_T, SIZE>;
public:
    np_queue_t() = default;;
    np_queue_t(std::function<void()>&& notify_handler,
               std::function<bool()>&& waiting_handler)
               : m_notify_handler(notify_handler),
                 m_waiting_handler(waiting_handler) {}
    np_queue_t(np_queue_t<ELE_T, SIZE>&& other) noexcept {
        m_is_notify_mode.store(other.m_is_notify_mode);
        m_notified.store(other.m_notified);
        m_notify_handler = std::move(other.m_notify_handler);
        m_waiting_handler = std::move(other.m_waiting_handler);
        ELE_T tmp_ele;
        while (other.m_queue.try_pop(tmp_ele)) {
            m_queue.try_push(tmp_ele);
        }
    }
    void set_handlers(std::function<void()>&& notify_handler,
                      std::function<bool()>&& waiting_handler);
    bool try_enqueue(ELE_T&& p);
    bool enqueue(ELE_T&& p);
    bool dequeue(ELE_T& p);

    unsigned int get_size();

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

template <class ELE_T, unsigned SIZE>
void np_queue_t<ELE_T, SIZE>::set_handlers(
        std::function<void()> &&notify_handler,
        std::function<bool()> &&waiting_handler)
{
    m_notify_handler = notify_handler;
    m_waiting_handler = waiting_handler;
}

template <class ELE_T, unsigned SIZE>
int np_queue_t<ELE_T, SIZE>::notify()
{
    // 检查模式。notify 模式则通知
    if (m_is_notify_mode) {
//        if (!m_notified) {
//            // notify
//            if (m_notify_handler) {
//                // notify handler 需要是线程安全的
//                m_notify_handler();
//            } else {
//                return -1;
//            }
//            m_notified = true;
//        }
        if (m_notify_handler) {
            // notify handler 需要是线程安全的
            m_notify_handler();
        } else {
            return -1;
        }
        m_notified = true;
    }
    return 0;
}

template <class ELE_T, unsigned SIZE>
bool np_queue_t<ELE_T, SIZE>::enqueue(ELE_T&& p)
{
    // 1. 写入
    m_queue.push(p);
    // 2. 通知
    if (0 != notify()) {
        return true;
    }
    return true;
}

template <class ELE_T, unsigned SIZE>
bool np_queue_t<ELE_T, SIZE>::try_enqueue(ELE_T&& p)
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

template <class ELE_T, unsigned SIZE>
bool np_queue_t<ELE_T, SIZE>::dequeue(ELE_T& p)
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

template <class ELE_T, unsigned SIZE>
unsigned int np_queue_t<ELE_T, SIZE>::get_size()
{
    return m_queue.was_size();
}

#endif //CPP_TOOLKIT_MOD_NP_QUEUE_HPP
