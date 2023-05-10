#include <mod_time_wheel/mod_time_wheel.hpp>


time_wheel_task_t::time_wheel_task_t(int timeout_interval, timer_callback_t callback)
        : m_timeout_interval(timeout_interval), m_callback(std::move(callback)) {}

time_wheel_task_t::time_wheel_task_t(int timeout_interval, size_t cycles, timer_callback_t callback)
        : m_timeout_interval(timeout_interval), m_remaining_cycles(cycles), m_callback(std::move(callback)) {}

void time_wheel_task_t::tick() {
    m_callback();
//    if (m_remaining_cycles == 0) {
//        m_callback();
//    } else {
//        m_remaining_cycles--;
//    }
}

size_t time_wheel_task_t::get_timeout_interval() const {
    return m_timeout_interval;
}

time_wheel_t::time_wheel_t(size_t slots) : m_slots(slots), m_current_slot(0), m_tasks(slots) {}

void time_wheel_t::add_tw_task(const time_wheel_task_t& timer) {
    auto timeout_interval = timer.get_timeout_interval();
    {
        std::lock_guard lock(m_mutex_tasks);
        size_t slot = (m_current_slot + timeout_interval) % m_slots;
        m_tasks[slot].push_back(timer);
    }
}

void time_wheel_t::tick() {
    std::vector<time_wheel_task_t> expired_timers;
    {
        std::lock_guard lock(m_mutex_tasks);
        expired_timers = std::move(m_tasks[m_current_slot]);
        m_current_slot = (m_current_slot + 1) % m_slots;
    }
    for (auto& timer : expired_timers) {
        timer.tick();
    }
}
