#include <iostream>

#include "ftxui/component/captured_mouse.hpp"  // for ftxui
#include "ftxui/component/component.hpp"       // for Input, Renderer, Vertical
#include "ftxui/component/component_base.hpp"  // for ComponentBase
#include "ftxui/component/component_options.hpp"  // for InputOption
#include "ftxui/component/screen_interactive.hpp"  // for Component, ScreenInteractive
#include "ftxui/dom/elements.hpp"  // for text, hbox, separator, Element, operator|, vbox, border
#include "ftxui/util/ref.hpp"  // for Ref

static int test_button(int argc, const char* argv[]) {
    using namespace ftxui;
    int value = 50;

    // The tree of components. This defines how to navigate using the keyboard.
    auto buttons = Container::Horizontal({
                                                 Button("Decrease", [&] { value--; }),
                                                 Button("Increase", [&] { value++; }),
                                         });

    // Modify the way to render them on screen:
    auto component = Renderer(buttons, [&] {
        return vbox({
                            text("value = " + std::to_string(value)),
                            separator(),
                            gauge(value * 0.01f),
                            separator(),
                            buttons->Render(),
                    }) |
               border;
    });

    auto screen = ScreenInteractive::FitComponent();
    screen.Loop(component);
    return 0;
}

static int test_window(int argc, const char* argv[]) {
    using namespace ftxui;
    Element document = graph([](int x, int y) {
        std::vector<int> result(x, 0);
        for (int i{0}; i < x; ++i) {
            result[i] = ((3 * i) / 2) % y;
        }
        return result;
    });

    document |= color(Color::Red);
    document |= bgcolor(Color::DarkBlue);
    document |= border;

    const int width = 80;
    const int height = 10;
    auto screen =
            Screen::Create(Dimension::Fixed(width), Dimension::Fixed(height));
    Render(screen, document);
    screen.Print();
    return EXIT_SUCCESS;
}

static int test_html_like(int argc, const char* argv[]) {
    using namespace ftxui;
    using namespace std::chrono_literals;

    auto img1 = []() { return text("img") | border; };
    auto img2 = []() { return vbox({text("big"), text("image")}) | border; };

    std::string reset_position;
    for (int i = 0;; ++i) {
        auto document =  //
                hflow(
                        paragraph("Hello world! Here is an image:"), img1(),
                        paragraph(" Here is a text "), text("underlined ") | underlined,
                        paragraph(" Here is a text "), text("bold ") | bold,
                        paragraph("Hello world! Here is an image:"), img2(),
                        paragraph(
                                "Le Lorem Ipsum est simplement du faux texte employé dans la "
                                "composition et la mise en page avant impression. Le Lorem "
                                "Ipsum est le faux texte standard de l'imprimerie depuis les "
                                "années 1500, quand un imprimeur anonyme assembla ensemble "
                                "des morceaux de texte pour réaliser un livre spécimen de "
                                "polices de texte. Il n'a pas fait que survivre cinq siècles, "
                                "mais s'est aussi adapté à la bureautique informatique, sans "
                                "que son contenu n'en soit modifié. Il a été popularisé dans "
                                "les années 1960 grâce à la vente de feuilles Letraset "
                                "contenant des passages du Lorem Ipsum, et, plus récemment, "
                                "par son inclusion dans des applications de mise en page de "
                                "texte, comme Aldus PageMaker."),
                        paragraph(" Here is a text "), text("dim ") | dim,
                        paragraph("Hello world! Here is an image:"), img1(),
                        paragraph(" Here is a text "), text("red ") | color(Color::Red),
                        paragraph(" A spinner "), spinner(6, i / 10)) |
                border;

        auto screen = Screen::Create(Dimension::Fit(document));
        Render(screen, document);
        std::cout << reset_position;
        screen.Print();
        reset_position = screen.ResetPosition();

        std::this_thread::sleep_for(0.01s);
    }

    return 0;
}

static int test_input2(int argc, const char* argv[])
{
    using namespace ftxui;

    std::string input_msg;
    Component msg_input_box;
    Elements display_msgs;
    float text_area_msg_cnt = 0;

    // 输入框
    InputOption input_option;
    input_option.on_enter = [&](){
        if (!input_msg.empty()) {
            display_msgs.push_back(hflow(paragraph(input_msg)));
            text_area_msg_cnt += 1;
            input_msg.clear();
        }
    };
    msg_input_box = Input(&input_msg, "", input_option);

    // 显示消息
    float taxt_area_cur_page = 0;
    auto get_text_area_pos = [&](){
        if (text_area_msg_cnt <= 0) {
            text_area_msg_cnt = 0;
            return .0f;
        }
        float h_per_line = 1.0f / text_area_msg_cnt;
        float h_page = 10 * h_per_line;
        float max_page_num = text_area_msg_cnt / 10;
        if (taxt_area_cur_page > max_page_num) {
            taxt_area_cur_page = max_page_num;
        }
        return (1.0f - (taxt_area_cur_page * h_page));
    };
    auto get_text_area = [&](){
        return vbox({ display_msgs });
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

    auto screen = ScreenInteractive::Fullscreen();
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
    screen.Loop(root_comp);

    return 0;
}

static int test_input(int argc, const char* argv[]) {
    using namespace ftxui;

    std::string first_name;
    std::string last_name;
    std::string password;

    Component input_first_name = Input(&first_name, "first name");
    Component input_last_name = Input(&last_name, "last name");

    InputOption password_option;
    password_option.password = true;
    Component input_password = Input(&password, "password", password_option);

    auto component = Container::Vertical({
                                                 input_first_name,
                                                 input_last_name,
                                                 input_password,
                                         });

    auto renderer = Renderer(component, [&] {
        return vbox({
                            text("Hello " + first_name + " " + last_name),
                            separator(),
                            hbox(text(" First name : "), input_first_name->Render()),
                            hbox(text(" Last name  : "), input_last_name->Render()),
                            hbox(text(" Password   : "), input_password->Render()),
                    }) |
               border;
    });

    auto screen = ScreenInteractive::TerminalOutput();
    screen.Loop(renderer);

    return 0;
}

static int test_pixel(int argc, const char* argv[])
{
    using namespace ftxui;
    auto screen = Screen::Create(Dimension::Fixed(32), Dimension::Fixed(10));

    auto& pixel = screen.PixelAt(9,9);
    pixel.character = U'A';
    pixel.bold = true;
    pixel.foreground_color = Color::Blue;

    std::cout << screen.ToString();
    return EXIT_SUCCESS;
}

static int test_box(int argc, const char* argv[])
{
    using namespace ftxui;

    // Define the document
    Element document =
            hbox({
                         text("left")   | border,
                         text("middle") | border | flex,
                         text("right")  | border,
                 });

    auto screen = Screen::Create(
            Dimension::Full(),       // Width
            Dimension::Fit(document) // Height
    );
    Render(screen, document);
    screen.Print();

    return EXIT_SUCCESS;
}

static int test_co(int argc, const char* argv[])
{

    return 0;
}

static int program_main(int argc, const char* argv[])
{
    int ret = -1;
//    ret = test_box(argc, argv);
//    ret = test_pixel(argc, argv);
//    ret = test_input(argc, argv);
//    ret = test_input2(argc, argv);
//    ret = test_html_like(argc, argv);
//    ret = test_window(argc, argv);
//    ret = test_button(argc, argv);
    ret = test_co(argc, argv);
    return ret;
}

int main(int argc, const char* argv[])
{
    int ret = -1;

    try {
        ret = program_main(argc, argv);
    } catch (const std::overflow_error& e) {
        std::cout << "overflow_error: " << e.what();
    } catch (const std::runtime_error& e) {
        std::cout << "runtime_error: " << e.what();
    } catch (const std::exception& e) {
        std::cout << "exception: " << e.what();
    } catch (...) {
        std::cout << "unknown exception!";
    }

    return ret;
}
