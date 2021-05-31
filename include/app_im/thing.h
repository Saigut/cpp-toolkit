#ifndef CPP_TOOLKIT_THING_H
#define CPP_TOOLKIT_THING_H

#include <mod_common/log.h>
#include <memory>
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/context/continuation.hpp>

namespace context = boost::context;

class Thing;
context::continuation thing(context::continuation && c, class Thing* c_thing);

class Thing {
public:
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
//            boost::function<context::continuation(context::continuation&&)> func_thing;
//            func_thing = std::move(boost::bind(&Thing::thing, this, _1));
            m_thing_point = context::callcc([t_this](context::continuation && c){
                log_info("my thing A");
                c = t_this->other_thing(std::move(c));
                log_info("my thing C");
                log_info("my thing D");
                t_this->stopped = true;
                c = c.resume();
                return std::move(c);
            });
//            m_thing_point = context::callcc(func_thing);
        } else {
            m_thing_point = m_thing_point.resume();
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

    context::continuation thing(context::continuation && c) {
        // tell other
        // get point of after 3, thing_point = that point
        // goto point;
        log_info("my thing A");
        c = other_thing(std::move(c));
        log_info("my thing C");
        log_info("my thing D");
        stopped = true;
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

    context::continuation m_thing_point;
    // func_point  thing_point;
    // thing context;

    bool waiting = false;
    bool began = false;
    bool stopped = false;
};

context::continuation thing(context::continuation && c, class Thing* c_thing) {
    // tell other
    // get point of after 3, thing_point = that point
    // goto point;
    log_info("my thing A");
    c = c_thing->other_thing(std::move(c));
    log_info("my thing C");
    log_info("my thing D");
    c_thing->stopped = true;
    c = c.resume();
    return std::move(c);
}



typedef std::shared_ptr<Thing> SP_Thing_t;
#define New_SP_Thing_t() std::make_shared<Thing>();

//class Thing_ImSend : public Thing {
//public:
//private:
//
//};
//
//typedef std::shared_ptr<Thing_ImSend> SP_Thing_ImSend_t;
//#define New_SP_Thing_ImSend_t() std::make_shared<Thing_ImSend>();


#endif //CPP_TOOLKIT_THING_H
