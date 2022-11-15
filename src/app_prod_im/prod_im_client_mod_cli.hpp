#ifndef CPP_TOOLKIT_PROD_IM_CLIENT_MOD_CLI_HPP
#define CPP_TOOLKIT_PROD_IM_CLIENT_MOD_CLI_HPP

#include <string>

int prod_im_client_mod_cli_read_loop();
void prod_im_client_mod_cli_recv_msg(std::string&& msg);

static void print_client_cli_usage()
{
    prod_im_client_mod_cli_recv_msg("-| REPL usage eg:\n");
    prod_im_client_mod_cli_recv_msg("-| reg <user passwd>\n");
    prod_im_client_mod_cli_recv_msg("-| login <user passwd>\n");
    prod_im_client_mod_cli_recv_msg("-| cont_list\n");
    prod_im_client_mod_cli_recv_msg("-| cont_add <contact id> <contact name>\n");
    prod_im_client_mod_cli_recv_msg("-| cont_del <contact id>\n");
    prod_im_client_mod_cli_recv_msg("-| msg <contact id> <message>\n");
}

#endif //CPP_TOOLKIT_PROD_IM_CLIENT_MOD_CLI_HPP
