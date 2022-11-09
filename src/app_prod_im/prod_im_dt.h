#ifndef CPP_TOOLKIT_PROD_IM_DT_H
#define CPP_TOOLKIT_PROD_IM_DT_H

#include <string>

struct prod_im_contact {
    std::string contact_id;
    std::string contact_name;
};
using prod_im_cont_list = std::vector<prod_im_contact>;

struct prod_im_user_account {
    std::string user_id;
    std::string user_pass;
};

struct prod_im_s_client_info {
    std::string client_ip;
    uint16_t client_port;
};

struct prod_im_chat_msg {
    std::string sender_id;
    std::string receiver_id;
    std::string chat_msg;
};


#endif //CPP_TOOLKIT_PROD_IM_DT_H
