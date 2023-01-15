#ifndef CPP_TOOLKIT_MOD_NETLIB_TCP_H
#define CPP_TOOLKIT_MOD_NETLIB_TCP_H

#include "mod_netlib.hpp"

class cppt_netlib_tcp : public cppt_netlib {
public:
    int connect(void* peer_addr, size_t addr_len, uint16_t peer_port,
                        std::function<void(void*)> callback) override {
        return -1;
    }
    int accept(std::function<void(void*)> callback) override {
        return -1;
    }
    int close_conn(void* conn_handler) override {
        return -1;
    }

    int create_stream(void* conn_handler,
                              std::function<void(void*)> callback) override {
        return -1;
    }
    int accept_stream(std::function<void(void*)> callback) override {
        return -1;
    }
    int close_stream(xqc_stream_t* stream) override {
        return -1;
    }

    int stream_read(xqc_stream_t* stream,
                            std::function<void(void*)> callback) override {
        return -1;
    }
    int stream_write(xqc_stream_t* stream,
                             const uint8_t* buf, size_t data_sz,
                             std::function<void(void*)> callback) override {
        return -1;
    }
};

#endif //CPP_TOOLKIT_MOD_NETLIB_TCP_H
