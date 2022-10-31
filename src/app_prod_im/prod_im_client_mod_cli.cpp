#include "app_prod_im_internal.h"

#include <thread>
#include <sstream>
#include <ncursesw/curses.h>
#include <mod_common/expect.h>
#include <mod_common/utils.h>


extern std::shared_ptr<prod_im_c_mod_main> g_client_main;


static int cli_reg(std::string& passwd)
{
    return g_client_main->user_register(passwd);
}

static int cli_login(std::string& passwd)
{
    return g_client_main->login(passwd);
}

static int cli_cont_list()
{
    auto contact_list = g_client_main->get_contact_list();
    if (contact_list->empty()) {
        prod_im_client_mod_cli_recv_msg("-| Empty contact list.\n");
        return 0;
    }

    int cnt = 1;
    for (auto& item : *contact_list) {
        prod_im_client_mod_cli_recv_msg("-| Contact %d:\n", cnt);
        prod_im_client_mod_cli_recv_msg("-| Id: %s\n", item.contact_id.c_str());
        prod_im_client_mod_cli_recv_msg("-| Name: %s\n", item.contact_name.c_str());
    }

    return 0;
}

static int cli_cont_add(std::string& contact_id, std::string& contact_name)
{
    return g_client_main->add_contact(contact_id, contact_name);
}

static int cli_cont_del(std::string& contact_id)
{
    g_client_main->del_contact(contact_id);
    return 0;
}

static int cli_msg(std::vector<std::string>& tokens)
{
    std::string& contact_id = tokens[1];
    std::string message = tokens[2];
    size_t i;
    for (i = 3; i < tokens.size(); i++) {
        message += " ";
        message += tokens[i];
    }
    int ret = g_client_main->send_msg_to_contact(contact_id, message);
    if (0 == ret) {
        prod_im_client_mod_cli_recv_msg("<me>: %s\n", message.c_str());
    }
    return ret;
}

//int prod_im_client_mod_cli(int argc, char** argv)
//{
//    const char* cmd;
//    std::string param_1;
//    std::string param_2;
//
//    expect_goto(argc >= 1, fail_return);
//    cmd = argv[0];
//    if (argc >= 2) {
//        param_1 = argv[1];
//    }
//    if (argc >= 3) {
//        param_2 = argv[2];
//    }
//
//    if (0 == strcmp("reg", cmd)) {
//        expect_goto(argc >= 3, fail_return);
//        return cli_reg(param_1, param_2);
//
//    } else if (0 == strcmp("login", cmd)) {
//        expect_goto(argc >= 3, fail_return);
//        return cli_login(param_1, param_2);
//
//    } else if (0 == strcmp("cont_list", cmd)) {
//        expect_goto(argc >= 2, fail_return);
//        return cli_cont_list(param_1);
//
//    }  else if (0 == strcmp("cont_add", cmd)) {
//        expect_goto(argc >= 3, fail_return);
//        return cli_cont_add(param_1, param_2);
//
//    }  else if (0 == strcmp("cont_del", cmd)) {
//        expect_goto(argc >= 2, fail_return);
//        return cli_cont_del(param_1);
//
//    }  else if (0 == strcmp("msg", cmd)) {
//        expect_goto(argc >= 3, fail_return);
//        return cli_msg(param_1, param_2);
//
//    } else {
//        log_error("Unknown cmd: %s", cmd);
//        print_client_cli_usage();
//        return -1;
//    }
//
//fail_return:
//    print_client_cli_usage();
//    return -1;
//}

static int process_read_in(std::vector<std::string> tokens)
{
    if (tokens.empty()) {
        print_client_cli_usage();
        return -1;
    }
    size_t tokens_number = tokens.size();
    std::string& cmd = tokens[0];

    if ("reg" == cmd) {
        expect_goto(tokens_number >= 2, fail_return);
        return cli_reg(tokens[1]);

    } else if ("login" == cmd) {
        expect_goto(tokens_number >= 2, fail_return);
        return cli_login(tokens[1]);

    } else if ("cont_list" == cmd) {
        expect_goto(tokens_number >= 1, fail_return);
        return cli_cont_list();

    }  else if ("cont_add" == cmd) {
        expect_goto(tokens_number >= 3, fail_return);
        return cli_cont_add(tokens[1], tokens[2]);

    }  else if ("cont_del" == cmd) {
        expect_goto(tokens_number >= 2, fail_return);
        return cli_cont_del(tokens[1]);

    }  else if ("msg" == cmd) {
        expect_goto(tokens_number >= 3, fail_return);
        return cli_msg(tokens);

    } else {
        prod_im_client_mod_cli_recv_msg("-| Unknown cmd: %s\n", cmd.c_str());
        print_client_cli_usage();
        return -1;
    }

fail_return:
    print_client_cli_usage();
    return -1;
}

int prod_im_client_mod_cli_read_loop_old()
{
    std::string input_s;
    std::string token;
    printf(">> ");
    while (std::getline(std::cin, input_s)) {
        if (!input_s.empty()) {
            std::stringstream ss(input_s);
            std::vector<std::string> tokens;
            while (ss >> token) {
                tokens.push_back(token);
            }
            process_read_in(tokens);
        }
        printf(">> ");
    }
    return 0;
}

static WINDOW* win;
static WINDOW* output_win;
static WINDOW* input_win;
static int row = 0, col = 0;
static std::atomic<bool> flag(false);
static std::string buf;

static void ninit()
{
    win = initscr();
    getmaxyx(win, row, col);

    cbreak();
    noecho();

    nonl();
    intrflush(stdscr, FALSE);
    keypad(stdscr, TRUE);
    refresh();
}

static void nprintf(std::string str)
{
    touchwin(win);
    str += '\n';
    wprintw(output_win, str.c_str());
    wrefresh(output_win);
    wrefresh(input_win);
}

void prod_im_client_mod_cli_recv_msg(const char* fmt, ...)
{
    touchwin(win);

    va_list ap;
    va_start(ap, fmt);
    vw_printw(output_win, fmt, ap);
    va_end(ap);

    wrefresh(output_win);
    wrefresh(input_win);
}

static void nmonitor()
{
    while(true)
    {
        char x = wgetch(input_win);

        if(x != '\r')
        {
            touchwin(win);
            buf += x;
            waddch(input_win, x);
        }
        else
        {
//            nprintf(buf);
            touchwin(input_win);
            flag = true;
            wclear(input_win);
        }
        wrefresh(input_win);
    }
}

static std::string nget()
{
    while(!flag) {
        cppt_usleep(100);
    }
    std::string cmd = buf;
    flag = false;
    buf = "";
    return cmd;
}


int prod_im_client_mod_cli_read_loop()
{
    ninit();
    fflush(stdin);

    output_win = subwin(win, row - 1, col, 0, 0);
    scrollok(output_win, true);
    input_win = subwin(win, 1, col, row - 1, 0);

    std::thread nthr(nmonitor);

    std::string input_s;
    std::string token;
    while (true)
    {
        input_s = nget();
        if(input_s == "quit") {
            break;
        } else {
            if (!input_s.empty()) {
                std::stringstream ss(input_s);
                std::vector<std::string> tokens;
                while (ss >> token) {
                    tokens.push_back(token);
                }
                process_read_in(tokens);
//                prod_im_client_mod_cli_recv_msg(">> %s\n", input_s.c_str());
            }
        }
    }

    getch();

    endwin();
    return 0;
}
