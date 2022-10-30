#include "app_prod_im_internal.h"

#include <mod_common/expect.h>


extern std::shared_ptr<prod_im_c_mod_main> g_client_main;


static int cli_reg(const char* user_id, const char* passwd)
{
    std::string str_user_id = user_id;
    std::string str_passwd = passwd;
    return g_client_main->user_register(str_user_id, str_passwd);
}

static int cli_login(const char* user_id, const char* passwd)
{
    std::string str_user_id = user_id;
    std::string str_passwd = passwd;
    return g_client_main->login(str_user_id, str_passwd);
}

static int cli_cont_list(const char* user_id)
{
    std::string str_user_id = user_id;
    auto contact_list = g_client_main->get_contact_list();
    if (contact_list->empty()) {
        printf("Empty contact list.\n");
        return 0;
    }

    int cnt = 1;
    for (auto& item : *contact_list) {
        printf("Contact %d:\n", cnt);
        printf("Id: %s\n", item.contact_id.c_str());
        printf("Name: %s\n", item.contact_name.c_str());
    }

    return 0;
}

static int cli_cont_add(const char* contact_id, const char* contact_name)
{
    std::string str_contact_id = contact_id;
    std::string str_contact_name = contact_name;
    return g_client_main->add_contact(str_contact_id,
                                      str_contact_name);
}

static int cli_cont_del(const char* contact_id)
{
    std::string str_contact_id = contact_id;
    g_client_main->del_contact(str_contact_id);
    return 0;
}

static int cli_msg(const char* contact_id, const char* message)
{
    std::string str_contact_id = contact_id;
    std::string str_message = message;
    return g_client_main->send_msg_to_contact(str_contact_id, str_message);
}

int prod_im_client_mod_cli(int argc, char** argv)
{
    const char* cmd;

    expect_goto(argc >= 1, fail_return);
    cmd = argv[0];

    if (0 == strcmp("reg", cmd)) {
        expect_goto(argc >= 3, fail_return);
        return cli_reg(argv[1], argv[2]);

    } else if (0 == strcmp("login", cmd)) {
        expect_goto(argc >= 3, fail_return);
        return cli_login(argv[1], argv[2]);

    } else if (0 == strcmp("cont_list", cmd)) {
        expect_goto(argc >= 2, fail_return);
        return cli_cont_list(argv[1]);

    }  else if (0 == strcmp("cont_add", cmd)) {
        expect_goto(argc >= 3, fail_return);
        return cli_cont_add(argv[1], argv[2]);

    }  else if (0 == strcmp("cont_del", cmd)) {
        expect_goto(argc >= 2, fail_return);
        return cli_cont_del(argv[1]);

    }  else if (0 == strcmp("msg", cmd)) {
        expect_goto(argc >= 3, fail_return);
        return cli_msg(argv[1], argv[2]);

    } else {
        log_error("Unknown cmd: %s", cmd);
        print_client_cli_usage();
        return -1;
    }

fail_return:
    print_client_cli_usage();
    return -1;
}
