#ifndef CPP_TOOLKIT_APP_PROD_IM_INTERNAL_H
#define CPP_TOOLKIT_APP_PROD_IM_INTERNAL_H

int app_prod_im_server(int argc, char** argv);
int app_prod_im_client(int argc, char** argv);


// common
struct prod_im_contact {
    std::string contact_id;
    std::string contact_name;
};



// client



// server
struct prod_im_s_user_session {
    std::string user_id;
    int io_port;
};

class prod_im_s_mod_uinfo {
public:
    int user_add(std::string& user_id, std::string& user_pass);

    void user_del(std::string& user_id);

    int user_contact_add(std::string& user_id,
                         std::string& contact_id,
                         std::string& contact_name);

    void user_contact_del(std::string& user_id, std::string& contact_id);

    std::vector <prod_im_contact>&& user_contact_get_list(std::string& user_id);
};

class prod_im_s_mod_user_session {
public:
    int add(std::string& user_id);
    void del(std::string& user_id);
    prod_im_s_user_session&& find(std::string& user_id);
};

class prod_im_s_mod_chat_msg_receiving {
public:
    void run();
};

class prod_im_s_mod_chat_msg_relay {
public:
    int relay_msg(std::string& sender_id,
                  std::string& receiver_id,
                  std::string& chat_content);
};


#endif //CPP_TOOLKIT_APP_PROD_IM_INTERNAL_H
