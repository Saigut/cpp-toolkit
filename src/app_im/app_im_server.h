#ifndef CPP_TOOLKIT_APP_IM_SERVER_H
#define CPP_TOOLKIT_APP_IM_SERVER_H

#include <cpt_im.grpc.pb.h>
#include <mod_queue/mod_queue.h>
#include <mod_hash_table/mod_hash_table.h>

#ifdef __cplusplus
extern "C" {
#endif

int app_im_server(int argc, char** argv);
int app_im_server_new(int argc, char** argv);
int app_im2_server(int argc, char** argv);

class im_server_base {
public:
    virtual void recv_msg(uint64_t& id, std::string& msg) = 0;
    virtual void relay_msg(uint64_t id, std::string& msg) = 0;
};

class im_server {
public:
    int init();
    void start();
    void grpc_receive();
    void grpc_send();

private:
    int deal_with_msg(const ::cpt_im::ServerIntfReq& req);

    friend class CptImServerServiceImpl;
    cpt_queue<::cpt_im::ServerIntfReq> r_queue;
    cpt_queue<::cpt_im::ClientIntfReq> s_queue;

    cpt_hash_table<uint64_t, std::string> uid_uport_table;
};


#ifdef __cplusplus
}
#endif

#endif //CPP_TOOLKIT_APP_IM_SERVER_H
