#include <iostream>

#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/screen.hpp>


static int test_pixel(int argc, char** argv)
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

static int test_box(int argc, char** argv)
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

static int program_main(int argc, char** argv)
{
    int ret = -1;
    ret = test_box(argc, argv);
    return ret;
}

int main(int argc, char** argv)
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
