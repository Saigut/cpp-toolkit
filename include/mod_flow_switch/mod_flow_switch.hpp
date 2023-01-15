#ifndef CPP_TOOLKIT_MOD_FLOW_SWITCH_HPP
#define CPP_TOOLKIT_MOD_FLOW_SWITCH_HPP

#include <stddef.h>
#include <vector>
#include <memory>
#include <mutex>

#include <mod_common/log.hpp>
#include <mod_ring_queue/mod_ring_queue.hpp>


class flow_switch {
public:

    virtual int init() {
        return 0;
    }
    virtual int deinit() {
        return 0;
    }

    int add_upper_io_rq(std::shared_ptr<ring_queue_rw> rq) {
        m_upper_io_rqs.push_back(rq);
        return 0;
    }
    int add_lower_io_rq(std::shared_ptr<ring_queue_rw> rq) {
        m_lower_io_rqs.push_back(rq);
        return 0;
    }
    virtual int flow_switch_step() { return -1; }
    virtual void print_stats() { }

protected:
    std::vector<std::shared_ptr<ring_queue_rw>> m_upper_io_rqs;
    std::vector<std::shared_ptr<ring_queue_rw>> m_lower_io_rqs;
};


#endif //CPP_TOOLKIT_MOD_FLOW_SWITCH_HPP
