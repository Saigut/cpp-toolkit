#ifndef CPP_TOOLKIT_MOD_COR_CHANNEL_HPP
#define CPP_TOOLKIT_MOD_COR_CHANNEL_HPP

#include <mod_np_queue/mod_np_queue.hpp>

#include "mod_cor.hpp"
#include "mod_cor_mutex.hpp"


namespace cppt {

    template <class ELE_T, unsigned SIZE = 2048>
    class cor_channel {
    public:
        explicit cor_channel();
        bool read(ELE_T& msg);
        bool write(ELE_T&& msg);

    private:
        np_queue_t<ELE_T, SIZE> m_q;
        cppt::cor_mutex_t m_cor_mutex;
        std::function<void()> m_resume_f = nullptr;
        bool m_notified = false;
    };

    template<class ELE_T, unsigned int SIZE>
    cor_channel<ELE_T, SIZE>::cor_channel() {
        auto notify_handler =
                [&]() {
                    m_notified = true;
                    m_cor_mutex.lock();
                    if (m_resume_f) {
                        m_resume_f();
                        m_resume_f = nullptr;
                    }
                    m_cor_mutex.unlock();
                };
        auto wait_handler =
                [&]() {
                    m_cor_mutex.lock();
                    auto wrap_func = [&](std::function<void()>&& resume_f) {
                        m_resume_f = std::move(resume_f);
                        if (m_notified) {
                            m_resume_f();
                            m_resume_f = nullptr;
                        }
                        m_cor_mutex.unlock();
                    };
                    cppt::cor_yield(wrap_func);
                    m_notified = false;
                    return true;
                };
        m_q.set_handlers(notify_handler, wait_handler);
    }

    template<class ELE_T, unsigned int SIZE>
    bool cor_channel<ELE_T, SIZE>::read(ELE_T& msg) {
        return m_q.dequeue(msg);
    }

    template<class ELE_T, unsigned int SIZE>
    bool cor_channel<ELE_T, SIZE>::write(ELE_T&& msg) {
        return m_q.enqueue(std::move(msg));
    }
}

#endif //CPP_TOOLKIT_MOD_COR_CHANNEL_HPP
