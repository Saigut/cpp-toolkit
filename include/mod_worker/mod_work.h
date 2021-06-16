#ifndef CPP_TOOLKIT_MOD_WORK_H
#define CPP_TOOLKIT_MOD_WORK_H

#include <mod_common/log.h>
#include <memory>
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/context/continuation.hpp>


namespace context = boost::context;

class Work;
class Work_GetStdin;
class Work_NetIn;
class Work_NetOut;
#include "mod_worker.h"


class WorkingPoint {
public:
    WorkingPoint() = default;
    bool yield(int ret) {
        if (m_wp) {
            yield_param = ret;
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
    int yield_param = 0;    // 0, ok; 1, timeout; < 0, yield with error
};

class Work {
public:
    Work() = default;
    explicit Work(Work* consignor_work)
    : m_consignor_work(consignor_work), m_my_msg_id(consignor_work->m_my_sub_work_msg_id)
    {}
    explicit Work(Work* consignor_work, bool free_me_after_finished)
    : m_consignor_work(consignor_work), m_my_msg_id(consignor_work->m_my_sub_work_msg_id),
      m_free_me_after_finished(free_me_after_finished)
    {}
    virtual void do_my_part(int yield_param);
    void set_main_worker(Worker* main_worker);
    void finish_handler(Work* sub_work, int sub_work_ret, bool ret_by_sub_work);
    uint64_t m_my_sub_work_msg_id = 0;
    uint64_t m_my_msg_id = 0;
protected:
    virtual void do_work();
    void add_self_back_to_main_worker(Work* sub_work, int sub_work_ret, bool ret_by_sub_work);

    Worker* m_main_worker = nullptr;
    WorkingPoint m_wp;

    Work* m_consignor_work = nullptr;

    bool began = false;
    bool stopped = false;
    const bool m_free_me_after_finished = false;
};



#endif //CPP_TOOLKIT_MOD_WORK_H
