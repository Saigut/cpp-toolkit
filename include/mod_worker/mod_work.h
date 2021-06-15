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
    bool yield() {
        if (m_wp) {
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
};

class Work {
public:
    Work() = default;
    explicit Work(Work* consignor_work) : m_consignor_work(consignor_work)
    {}
    virtual void do_my_part();
    void set_main_worker(Worker* main_worker);
    void finish_handler(Work* sub_work, int sub_work_ret, bool ret_by_sub_work);
    bool dealed_with_finish = false;
    int finish_ret = 0;
//    uint64_t sub_work_id = 0;
protected:
    virtual void do_work();
    void add_self_back_to_main_worker(Work* sub_work, int sub_work_ret, bool ret_by_sub_work);

    Worker* m_main_worker = nullptr;
    WorkingPoint m_wp;

    Work* m_consignor_work = nullptr;

    bool began = false;
    bool stopped = false;
};

struct WorkWrap {
    WorkWrap(Work* work, Work* sub_work, int sub_work_ret, bool ret_by_sub_work)
            : m_work(work), m_sub_work(sub_work),
              m_sub_work_ret(sub_work_ret), m_ret_by_sub_work(ret_by_sub_work)
    {}
    explicit WorkWrap(Work* work)
            : m_work(work), m_sub_work(nullptr),
              m_sub_work_ret(0), m_ret_by_sub_work(false)
    {}
    Work* m_work = nullptr;
    Work* m_sub_work = nullptr;
    int m_sub_work_ret = 0;
    bool m_ret_by_sub_work = false;
};


#endif //CPP_TOOLKIT_MOD_WORK_H
