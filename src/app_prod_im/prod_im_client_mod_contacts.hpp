#ifndef CPP_TOOLKIT_PROD_IM_CLIENT_MOD_CONTACTS_HPP
#define CPP_TOOLKIT_PROD_IM_CLIENT_MOD_CONTACTS_HPP

#include <string>
#include <map>
#include "prod_im_dt.hpp"

class prod_im_c_mod_contacts {
public:
    void update_list(prod_im_cont_list& contact_list);
    int add_contact(std::string& contact_id, std::string& contact_name);
    int del_contact(std::string& contact_id);
private:
    std::map<std::string, prod_im_contact> m_contacts;
};

class prod_im_c_chat_msg_sending {
public:
    int send_msg(const std::string& server_ip, uint16_t server_port, const std::string& sender_id,
                 const std::string& receiver_id, const std::string& msg_content);
};

#endif //CPP_TOOLKIT_PROD_IM_CLIENT_MOD_CONTACTS_HPP
