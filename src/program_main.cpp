#include <program_main.h>

#include <stdio.h>
#include <json-c/json.h>

#include <iostream>
#include <boost/format.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/yield.hpp>

#include <mod_socket/mod_socket.h>
#include <mod_common/log.h>
#include <app_socket/app_socket.h>
#include <app_chat/app_chat.h>
#include <app_asio_socket/app_asio_socket.h>

class Cro : boost::asio::coroutine {
public:
    void operator()() {
        reenter (this) {
            log_info("here");
//            yield socket_->async_read_some(buffer(*buffer_), *this);
            yield log_info("yield");
            yield log_info("yield2");
            log_info("here");
        }
    }
    boost::asio::ip::tcp::socket* socket_;
};

int test2()
{
    int i;
    int a;

    for (i = 0; i < 3; i++) {
        a = i;
        switch (a) {
            for (;;)
            case 999:
                if (true)
                    case 0: {log_info("dd %d", a); }
            case 99: {log_info("dd2 %d", a); }
        }
    }

    return 0;
}

int test()
{
    Cro c;
    c();
    c();

    return 0;
}

int program_main(int argc, char** argv)
{
    int ret = 0;
//    struct json_object * j_obj = json_tokener_parse("{'First': 'Hello', 'Second': 'world'}");
//    if (j_obj) {
//        printf("%s, %s!\n",
//               json_object_get_string(json_object_object_get(j_obj, "First")),
//               json_object_get_string(json_object_object_get(j_obj, "Second")));
//    }
//    std::cout << boost::format("%1% %2% %3% %2% %1% \n") % "11" % "22" % "333";
//    mod_socket();
//    app_socket(argc, argv);
//    ret = app_chat(argc, argv);
    ret = app_asio_socket(argc, argv);

    return ret;
}
