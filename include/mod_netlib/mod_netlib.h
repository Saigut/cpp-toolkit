#ifndef CPP_TOOLKIT_MOD_NETLIB_H
#define CPP_TOOLKIT_MOD_NETLIB_H

class cppt_netlib {
public:
    virtual int connect(void* peer_addr, size_t addr_len, uint16_t peer_port,
                        std::function<void(void*)> callback) = 0;
    virtual int accept(std::function<void(void*)> callback) = 0;
    virtual int close_conn(void* conn_handler) = 0;

    virtual int create_stream(void* conn_handler,
                              std::function<void(void*)> callback) = 0;
    virtual int accept_stream(std::function<void(void*)> callback) = 0;
    virtual int close_stream(xqc_stream_t* stream) = 0;

    virtual int stream_read(xqc_stream_t* stream,
                            std::function<void(void*)> callback) = 0;
    virtual int stream_write(xqc_stream_t* stream,
                             const uint8_t* buf, size_t data_sz,
                             std::function<void(void*)> callback) = 0;
};

#endif //CPP_TOOLKIT_MOD_NETLIB_H
