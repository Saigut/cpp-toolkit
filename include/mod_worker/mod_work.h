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
    void finish_handler();
protected:
    virtual void do_work();
    void add_self_back_to_main_worker();

    Worker* m_main_worker = nullptr;
    WorkingPoint m_wp;

    Work* m_consignor_work = nullptr;

    bool began = false;
    bool stopped = false;
};


#endif //CPP_TOOLKIT_MOD_WORK_H
