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
                m_notify_handler();
            }
            m_notified = true;
        }
    }
    return 0;
}

int np_queue_t::dequeue()
{
    void* p;
    // 1. 切换为 poll 模式
    m_is_notify_mode = false;
    // 2. poll
    while (m_queue.try_pop(p)) {
        // call handler
        if (m_dequeue_handler) {
            m_dequeue_handler(p);
        }
    }
    // 3. 切换为 notify 模式
    m_is_notify_mode = true;
    while (!m_notified && m_queue.try_pop(p)) {
        // call handler
        if (m_dequeue_handler) {
            m_dequeue_handler(p);
        }
    }
    return 0;
}

int np_queue_t::dequeue_wait()
{
    while (true) {
        dequeue();
        // 等待 condition 信号
    }
    return 0;
}

int np_queue_t::dequeue_co()
{
    while (true) {
        dequeue();
        // 协程等待信号
    }
    return 0;
}
