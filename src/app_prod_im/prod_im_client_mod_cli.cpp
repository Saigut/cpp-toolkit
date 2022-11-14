#include "app_prod_im_internal.h"

#include <thread>
#include <sstream>
#include <ftxui/component/captured_mouse.hpp>
#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>
#include <ftxui/util/ref.hpp>
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
    if (!contact_list || contact_list->empty()) {
        prod_im_client_mod_cli_recv_msg("-| Empty contact list.\n");
        return 0;
    }

    int cnt = 1;
    prod_im_client_mod_cli_recv_msg("-| Contacts: " + std::to_string(cnt) + "\n");
    for (auto& item : *contact_list) {
        prod_im_client_mod_cli_recv_msg(
                "-| " + std::to_string(cnt) + ": " + item.contact_id + ", " + item.contact_name + "\n");
        cnt++;
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
//    if (0 == ret) {
//        prod_im_client_mod_cli_recv_msg("<me>: %s\n", message.c_str());
//    }
    return ret;
}

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
        prod_im_client_mod_cli_recv_msg(
                "-| Unknown cmd: " + cmd + ", cmd str len: "
                + std::to_string(strlen(cmd.c_str())) + "\n");
        print_client_cli_usage();
        return -1;
    }

fail_return:
    print_client_cli_usage();
    return -1;
}


static ftxui::ScreenInteractive gs_screen = ftxui::ScreenInteractive::Fullscreen();
static std::list<std::string> gs_input_msgs;
static std::mutex gs_thread_lock;

void prod_im_client_mod_cli_recv_msg(std::string&& msg)
{
    std::lock_guard lock(gs_thread_lock);
    gs_input_msgs.push_back(msg);
    gs_screen.PostEvent(ftxui::Event::Custom);
}

int prod_im_client_mod_cli_read_loop()
{
    using namespace ftxui;

    std::string input_msg;
    Elements gs_display_msgs;
    std::string token;
    Component msg_input_box;
    float text_area_msg_cnt = 0;

    // 输入框
    InputOption input_option;
    input_option.on_enter = [&](){
        if (!input_msg.empty()) {
//            display_msgs.push_back(hflow(paragraph(input_msg)));
//            text_area_msg_cnt += 1;

            std::stringstream ss(input_msg);
            std::vector<std::string> tokens;
            while (ss >> token) {
                tokens.push_back(token);
            }
            process_read_in(tokens);

            input_msg.clear();
        }
    };
    msg_input_box = Input(&input_msg, "", input_option);

    // 显示消息
    float taxt_area_cur_page = 0;
    float lines_of_one_age = 15;
    auto get_text_area_pos = [&](){
        if (text_area_msg_cnt <= 0) {
            text_area_msg_cnt = 0;
            return .0f;
        }
        float h_per_line = 1.0f / text_area_msg_cnt;
        float h_page = lines_of_one_age * h_per_line;
        float max_page_num = text_area_msg_cnt / lines_of_one_age;
        if (taxt_area_cur_page > max_page_num) {
            taxt_area_cur_page = max_page_num;
        }
        return (1.0f - (taxt_area_cur_page * h_page));
    };
    auto get_text_area = [&](){
        if (gs_thread_lock.try_lock()) {
            while (!gs_input_msgs.empty()) {
                auto msg = gs_input_msgs.front();
                gs_display_msgs.push_back(text(msg));
                text_area_msg_cnt += 1;
                gs_input_msgs.pop_front();
            }
            gs_thread_lock.unlock();
        }
        return vbox({ gs_display_msgs });
    };

    auto get_msg_disp_box = [&](){
        return vbox({
                            filler(),
                            get_text_area() | focusPositionRelative(0, get_text_area_pos()) | vscroll_indicator | frame,
                    });
    };

    auto component = Container::Vertical({
                                                 msg_input_box,
                                         });
    auto get_while_page = [&]() {
        return vbox({
                            get_msg_disp_box() | border | flex,
                            hbox(text("Input: "), msg_input_box->Render()) | border,
                    });
    };

//    auto screen = ScreenInteractive::Fullscreen();
    auto ui_root_comp = Renderer(component, [&] {
        return get_while_page();
    });
    auto root_comp = CatchEvent(ui_root_comp, [&](Event event) {
        if (event == Event::PageUp) {
            taxt_area_cur_page += 1;
            return true;
        } else if (event == Event::PageDown) {
            taxt_area_cur_page -= 1;
            if (taxt_area_cur_page < 0) {
                taxt_area_cur_page = 0;
            }
            return true;
        }
        return false;
    });
    gs_screen.Loop(root_comp);

    return 0;
}
