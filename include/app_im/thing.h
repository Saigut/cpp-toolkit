#ifndef CPP_TOOLKIT_THING_H
#define CPP_TOOLKIT_THING_H

#include <mod_common/log.h>
#include <memory>
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/context/continuation.hpp>
#include "worker.h"

namespace context = boost::context;



class Thing {
public:
    Thing() = default;
    explicit Thing(std::function<void(void)>&& wind_up_func) : m_wind_up_func(wind_up_func)
    {}
//    int do_main_part(boost::function<context::continuation(context::continuation&&)> func_thing) {
    int do_main_part() {
        if (stopped) {
            return 0;
        }
        if (waiting) {
            return -1;
        }
        if (!began) {
            began = true;
            class Thing* t_this = this;
            m_thing_point = context::callcc([t_this](context::continuation && c){
                c = t_this->thing(std::move(c));
                t_this->stopped = true;
                if (t_this->m_wind_up_func) {
                    t_this->m_wind_up_func();
                }
                return std::move(c);
            });
        } else {
            if (m_thing_point) {
                m_thing_point = m_thing_point.resume();
            }
        }
        return 0;
        // get point after thing();
        // goto thing_point whith point
        // this point;
    }
    int notify_other_part_done() {
        if (stopped) {
            return -1;
        }
        if (!waiting) {
            return -1;
        } else {
            waiting = false;
        }
        return 0;
    }

    virtual context::continuation thing(context::continuation && c) {
        // tell other
        // get point of after 3, thing_point = that point
        // goto point;
        log_info("my thing A");
        c = other_thing(std::move(c));
        log_info("my thing C");
        log_info("my thing D");
        stopped = true;
        if (m_wind_up_func) {
            m_wind_up_func();
        }
        c = c.resume();
        return std::move(c);
    }

//protected:
    context::continuation other_thing(context::continuation && c) {
        log_info("other thing B begin");
        waiting = true;
        c = c.resume();
        waiting = false;
        log_info("other thing B done");
        return std::move(c);
    }

    void set_main_worker(Worker<Thing>* main_worker) {
        m_main_worker = main_worker;
    }

    Worker<Thing>* m_main_worker;

    std::function<void(void)> m_wind_up_func;

    context::continuation m_thing_point;
    // func_point  thing_point;
    // thing context;

    bool waiting = false;
    bool began = false;
    bool stopped = false;
};
typedef std::shared_ptr<Thing> SP_Thing_t;
#define New_SP_Thing_t() std::make_shared<Thing>()

class Thing_Other : public Thing {
public:
    Thing_Other() = default;
    explicit Thing_Other(std::function<void(void)>&& wind_up_func)
            : Thing(std::move(wind_up_func))
    {}
//    explicit Thing_ImSend(std::function<void(void)>&& wind_up_func,
//                          std::shared_ptr<Worker<Thing>> other_worker)
//    : Thing(std::move(wind_up_func)), m_other_worker(other_worker)
//    {}

    context::continuation thing(context::continuation && c) override {
        log_info("before 2");
        std::this_thread::sleep_for(std::chrono::seconds (3));
        log_info("after 2");
//        if (m_wind_up_func) {
//            m_wind_up_func();
//        }
        return std::move(c);
    }
private:
//    std::shared_ptr<Worker<Thing>> m_other_worker;
    Worker<Thing>* m_other_worker;
};

class Thing_ImSend : public Thing {
public:
    Thing_ImSend() = default;
    explicit Thing_ImSend(std::function<void(void)>&& wind_up_func,
                          Worker<Thing>* other_worker)
    : Thing(std::move(wind_up_func)), m_other_worker(other_worker)
    {}
    explicit Thing_ImSend(Worker<Thing>* other_worker)
    : m_other_worker(other_worker)
    {}
//    explicit Thing_ImSend(std::function<void(void)>&& wind_up_func,
//                          std::shared_ptr<Worker<Thing>> other_worker)
//    : Thing(std::move(wind_up_func)), m_other_worker(other_worker)
//    {}

    void back_to_main_worker(void) {
//        waiting = false;
        m_main_worker->add_avail_thing(this);
    }

    context::continuation thing(context::continuation && c) override {
        log_info("1");
        std::function<void(void)> wind_up_func{std::bind(&Thing_ImSend::back_to_main_worker, this)};
//        wind_up_func = std::move();
        Thing_Other other{std::move(wind_up_func)};
        m_other_worker->add_avail_thing(&other);
//        waiting = true;
        c = c.resume();
        log_info("3");
        log_info("4");
//        c = c.resume();
        return std::move(c);
    }
private:
    Worker<Thing>* m_other_worker;
};

typedef std::shared_ptr<Thing_ImSend> SP_Thing_ImSend_t;
#define New_SP_Thing_ImSend_t() std::make_shared<Thing_ImSend>();


#endif //CPP_TOOLKIT_THING_H
