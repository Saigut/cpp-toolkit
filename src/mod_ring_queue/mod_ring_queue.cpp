#include <mod_ring_queue/mod_ring_queue.hpp>

#include <mod_common/log.hpp>
#include <mod_mempool/mod_mempool.hpp>


bool ring_queue::empty()
{
    return !front_ele && m_q.was_empty();
}

bool ring_queue::full()
{
    return m_q.was_size() >= m_capacity;
}

void ring_queue::front(void*& p)
{
    if (front_ele) {
        p = front_ele;
        return;
    }
    if (m_q.was_empty()) {
        p = nullptr;
    } else {
        p = m_q.pop();
        front_ele = p;
    }
}

int ring_queue::push(void* p)
{
    if (full()) {
//        log_error("full!");
        return -1;
    } else {
        m_q.push(p);
        return 0;
    }
}

int ring_queue::pop(void*& p)
{
    if (front_ele) {
        p = front_ele;
        front_ele = nullptr;
        return 0;
    }
    if (m_q.was_empty()) {
        log_error("empty!");
        return -1;
    } else {
        p = m_q.pop();
        return 0;
    }
}
