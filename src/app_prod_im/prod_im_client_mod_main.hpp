#ifndef CPP_TOOLKIT_PROD_IM_CLIENT_MOD_MAIN_HPP
#define CPP_TOOLKIT_PROD_IM_CLIENT_MOD_MAIN_HPP

#include <stdint.h>
#include <memory>
#include <string>

#include "prod_im_client_mod_grpc_api.hpp"
#include "prod_im_dt.h"

class prod_im_c_mod_main {
public:
    prod_im_c_mod_main(const std::string& mServerIp,
                       uint16_t mServerPort,
                       uint16_t mClientPort,
                       const std::string& mMyId,
                       std::shared_ptr<call_im_server_grpc> mServerGrpcApi)
            : m_server_ip(mServerIp),
              m_server_port(mServerPort),
              m_client_port(mClientPort),
              m_my_id(mMyId),
              m_server_grpc_api(mServerGrpcApi) {}

    int user_register(const std::string& user_pass);
    int login(const std::string& user_pass);
    std::shared_ptr<prod_im_cont_list> get_contact_list();
    int add_contact(const std::string& contact_id,
                    const std::string& contact_name);
    void del_contact(const std::string& contact_id);
    int send_msg_to_contact(const std::string& contact_id,
                            const std::string& chat_msg);
    void process_chat_msg(const std::string& sender_id,
                          const std::string& chat_msg);
    std::shared_ptr<prod_im_chat_msg_list> get_chat_msg();
    void run();
private:
    std::string m_server_ip;
    uint16_t m_server_port;
    uint16_t m_client_port;

    std::string m_my_id;
    size_t m_msg_index = 0;

    std::shared_ptr<call_im_server_grpc> m_server_grpc_api;
};

#endif //CPP_TOOLKIT_PROD_IM_CLIENT_MOD_MAIN_HPP
