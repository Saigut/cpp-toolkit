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


class Work {
public:
    Work() = default;
    explicit Work(Work* consignor) : m_consignor(consignor)
    {}
    virtual void do_my_part();
    void set_main_worker(Worker* main_worker);
    void consignor_add_self_back_to_main_worker();
protected:
    virtual void do_work();
    void add_self_back_to_main_worker();

    Worker* m_main_worker = nullptr;
    context::continuation m_wp;     // working point

    Work* m_consignor = nullptr;

    bool began = false;
    bool stopped = false;
};


#endif //CPP_TOOLKIT_MOD_WORK_H
