#include <iostream>
#include <app_coroutine/app_coroutine.h>

#define CPPHTTPLIB_OPENSSL_SUPPORT
#include <httplib/httplib.h>

static int test_cpp_httplib(int argc, char** argv)
{
    // HTTPS
    // openssl req -newkey rsa:2048 -new -nodes -x509 -days 3650 -keyout key.pem -out cert.pem
    httplib::SSLServer svr("./cert.pem", "./key.pem");

    svr.Get("/hi", [](const httplib::Request &, httplib::Response &res) {
        res.set_content("Hello World!", "text/plain");
    });

    svr.listen("0.0.0.0", 8080);
    return 0;
}

static int program_main(int argc, char** argv)
{
    int ret = 0;
//    ret = app_coroutine(argc, argv);
    ret = test_cpp_httplib(argc, argv);
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
