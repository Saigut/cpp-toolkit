#ifndef CPP_TOOLKIT_THING_H
#define CPP_TOOLKIT_THING_H

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
#include "worker.h"

typedef struct {
    uint8_t* buf;
    ssize_t buf_sz;
} buf_wrap;

class Work {
public:
    Work() = default;
    explicit Work(Work* consignor) : m_consignor(consignor)
    {}
    virtual void do_my_part();
    void set_my_worker(Worker* main_worker);
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

class Work_Other : public Work {
public:
    Work_Other() = default;
    explicit Work_Other(Work* consignor)
            : Work(consignor)
    {}
private:
    void do_work() override;
};

class Work_ImSend : public Work {
public:
    Work_ImSend() = default;
    explicit Work_ImSend(Worker_Net* other_worker)
    : m_other_worker(other_worker)
    {}
private:
    void do_work() override;
//    Worker* m_other_worker = nullptr;
    Worker_Net* m_other_worker = nullptr;
};

class Work_GetStdin : public Work {
public:
    Work_GetStdin() = default;
    explicit Work_GetStdin(Work* consignor)
            : Work(consignor)
    {}
    void do_my_part() override;
    uint8_t* data_buf = nullptr;
};

class Work_NetIn : public Work {
public:
    Work_NetIn() = default;
    explicit Work_NetIn(Work* consignor)
            : Work(consignor)
    {}
    void do_my_part() override;
    buf_wrap in_buf = {nullptr, 0};
    int fd = -1;
};

class Work_NetOut : public Work {
public:
    Work_NetOut() = default;
    explicit Work_NetOut(Work* consignor)
            : Work(consignor)
    {}
    void do_my_part() override;
    buf_wrap out_buf = {nullptr, 0};
    int fd = -1;
};

#endif //CPP_TOOLKIT_THING_H
