#ifndef CPP_TOOLKIT_MOD_TIME_WHEEL_HPP
#define CPP_TOOLKIT_MOD_TIME_WHEEL_HPP

#include <vector>
#include <mutex>
#include <functional>


class time_wheel_task_t {
public:
    using timer_callback_t = std::function<void()>;

    time_wheel_task_t(int timeout_interval, size_t cycles, timer_callback_t callback);
    time_wheel_task_t(int timeout_interval, timer_callback_t callback);

    void tick();
    size_t get_timeout_interval() const;

private:
    size_t m_timeout_interval;
    size_t m_remaining_cycles = 0;
    timer_callback_t m_callback;
};

class time_wheel_t {
public:
    explicit time_wheel_t(size_t slots);

    void add_tw_task(const time_wheel_task_t& timer);
    void tick();

private:
    size_t m_slots;
    size_t m_current_slot;
    std::mutex m_mutex_tasks;
    std::vector<std::vector<time_wheel_task_t>> m_tasks;
};


#endif //CPP_TOOLKIT_MOD_TIME_WHEEL_HPP
