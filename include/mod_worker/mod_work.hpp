#ifndef CPP_TOOLKIT_MOD_WORK_HPP
#define CPP_TOOLKIT_MOD_WORK_HPP

#include <mod_common/log.hpp>
#include <memory>
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/context/continuation.hpp>


namespace context = boost::context;

class Work;
class Work_GetStdin;
class Work_NetIn;
class Work_NetOut;
#include "mod_worker.hpp"


class WorkingPoint {
public:
    WorkingPoint() = default;
    bool wp_yield(int ret) {
        if (m_wp) {
            m_yield_param = ret;
            m_wp = m_wp.resume();
            return true;
        } else {
            return false;
        }
    }
    void set_wp(context::continuation&& wp) {
        m_wp = std::move(wp);
    }
    context::continuation m_wp;
    int m_yield_param = 0;    // 0, ok; 1, timeout; < 0, yield with error
};

class Work : public std::enable_shared_from_this<Work> {
public:
    Work() = default;
    explicit Work(std::shared_ptr<Work> consignor_work)
    : m_consignor_work(consignor_work), m_my_msg_id(consignor_work->m_my_sub_work_msg_id)
    {}
    virtual void do_my_part(int yield_param);
    void set_main_worker(Worker* main_worker);
    void finish_handler();
    uint64_t m_my_sub_work_msg_id = 0;
    uint64_t m_my_msg_id = 0;
    int m_my_finish_ret_val = -1;

    WorkingPoint m_wp;
    void add_self_back_to_main_worker(std::shared_ptr<Work> sub_work);
protected:
    virtual void do_work();

    // Worker 通过全局的方式获取？或者通过一个类似 io_context 无处不在的东西来获取？
    Worker* m_main_worker = nullptr;

    // 应该以闭包的方式和 finish handler 绑定在一起？
    std::shared_ptr<Work> m_consignor_work = nullptr;

    // co_wait 怎么实现呢？  想办法用 finish_handler 做？

    bool began = false;
    bool stopped = false;
};


#endif //CPP_TOOLKIT_MOD_WORK_HPP
