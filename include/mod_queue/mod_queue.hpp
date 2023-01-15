#ifndef CPP_TOOLKIT_MOD_QUEUE_HPP
#define CPP_TOOLKIT_MOD_QUEUE_HPP

#include <memory>
#include <queue>
#include <thread>
#include <mutex>

#include <mod_common/log.hpp>

using lock_guard = std::lock_guard<std::mutex>;

template<class T>
class cpt_queue {
public:
    cpt_queue() : queue() {}
    bool init();
    bool write(const T&);
    bool read(T&);
private:
    std::shared_ptr<std::queue<T>> queue;
    std::mutex m_mutex;
};

template<typename T>
bool cpt_queue<T>::init() {
    lock_guard lock(this->m_mutex);
    this->queue = std::make_shared
            <std::queue<T>>();
    return true;
}

template<typename T>
bool cpt_queue<T>::write(const T& val) {
    lock_guard lock(this->m_mutex);
    this->queue->push(val);
    return true;
}

template<typename T>
bool cpt_queue<T>::read(T& val) {
    lock_guard lock(this->m_mutex);
    if (this->queue->empty()) {
        return false;
    }
    val = this->queue->front();
    this->queue->pop();
    return true;
}


#endif //CPP_TOOLKIT_MOD_QUEUE_HPP
