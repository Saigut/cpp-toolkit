#ifndef CPP_TOOLKIT_PROD_IM_SERVER_MOD_USER_INFO_HPP
#define CPP_TOOLKIT_PROD_IM_SERVER_MOD_USER_INFO_HPP

#include <string>
#include <memory>
#include <map>
#include <vector>
#include "prod_im_dt.hpp"

class prod_im_s_mod_uinfo {
private:
    struct user_info_t {
        std::string user_id;
        std::string user_pass;
        std::map<std::string, prod_im_contact> user_contacts;
        std::vector<prod_im_chat_msg> user_chat_msg;
    };
public:
    int user_add(const std::string& user_id, const std::string& user_pass);
    void user_del(const std::string& user_id);
    std::shared_ptr<user_info_t> user_find(const std::string& user_id);
    bool user_exist(const std::string& user_id);
    // 0, ok; -1 error; -2 user not exist
    int user_get_chat_msg(const std::string& user_id, prod_im_chat_msg_list& chat_msg);
    // 0, ok; -1 error; -2 user not exist
    int user_get_chat_msg_from(const std::string& user_id,
                               size_t msg_index,
                               prod_im_chat_msg_list& chat_msg);
    // 0, ok; -1 error; -2 user not exist
    int user_add_msg(const std::string& user_id, prod_im_chat_msg& chat_msg);

    int user_contact_add(const std::string& user_id,
                         const std::string& contact_id,
                         const std::string& contact_name);

    void user_contact_del(const std::string& user_id, const std::string& contact_id);

    std::shared_ptr<prod_im_cont_list> user_contact_get_list(const std::string& user_id);
private:
    std::map<std::string, user_info_t> m_users;
};

#endif //CPP_TOOLKIT_PROD_IM_SERVER_MOD_USER_INFO_HPP
