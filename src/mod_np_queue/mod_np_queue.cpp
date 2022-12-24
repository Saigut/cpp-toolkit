#include <mod_np_queue/mod_np_queue.hpp>


int np_queue_t::enqueue(void* p)
{
    // 1. 写入
    if (!m_queue.try_push(p)) {
        return -1;
    }
    // 2. 检查模式。notify 模式则通知。
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

bool np_queue_t::do_dequeue()
{
    void* p;
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

int np_queue_t::do_dequeue_wait()
{
    if (!m_waiting_handler) {
        return -1;
    }

    do {
        if (!do_dequeue()) {
            return 0;
        }
    } while (m_waiting_handler());

    return 0;
}
