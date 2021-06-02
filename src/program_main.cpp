#include <program_main.h>

#include <stdio.h>
#include <json-c/json.h>

#include <iostream>
#include <chrono>
#include <thread>
#include <boost/format.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/yield.hpp>

#include <mod_common/log.h>
#include <mod_socket/mod_socket.h>
#include <mod_hash_table/mod_hash_table.h>
#include <app_socket/app_socket.h>
#include <app_chat/app_chat.h>
#include <app_asio_socket/app_asio_socket.h>
#include <app_im/app_im.h>
#include <app_im/work.h>

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

int test_reenter()
{
    Cro c;
    log_info("-----");
    c();
    log_info("-----");
    c();
    log_info("-----");

    return 0;
}

void test_timeout_hash_table()
{
    std::string out_str;
    cpt_hash_table<int, std::string> table;
    expect_ret(table.init());
    table.insert(1, "111");

    out_str = "nothing";
    table.find(1, out_str);
    std::cout << "1st: " << out_str << std::endl;
    std::this_thread::sleep_for (
            std::chrono::seconds (1));

    out_str = "nothing";
    table.timeout_update();
    table.find(1, out_str);
    std::cout << "2nd: " << out_str << std::endl;
    std::this_thread::sleep_for (
            std::chrono::seconds (5));

    out_str = "nothing";
    table.timeout_update();
    table.find(1, out_str);
    std::cout << "3rd: " << out_str << std::endl;
}


#include <cstdlib>

#include <boost/context/continuation.hpp>
#include <boost/context/fiber.hpp>

namespace ctx = boost::context;

ctx::continuation foo( ctx::continuation && c)
{
    do {
        std::cout << "foo\n";
    } while ( ( c = c.resume() ) );
    std::cout << "before return\n";
    return std::move( c);
}

int ctx_main() {
    ctx::continuation c = ctx::callcc( foo);
    do {
        std::cout << "bar\n";
    } while ( ( c = c.resume() ) );
    std::cout << "main: done" << std::endl;
    return EXIT_SUCCESS;
}

ctx::fiber bar( ctx::fiber && f)
{
    do {
        std::cout << "bar\n";
        f = std::move( f).resume();
    } while ( f);
    return std::move( f);
}

int fb_main()
{
    ctx::fiber f{ bar };
    do {
        std::cout << "foo\n";
        f = std::move( f).resume();
    } while ( f);
    std::cout << "main: done" << std::endl;
    return EXIT_SUCCESS;
}

void worker2_thread(Worker* worker)
{
    worker->run();
}

int test_worker()
{
    Worker worker1{};
    Worker_Net worker2{};

    // Add work to worker1
    Work_ImSend thing{&worker2};
    worker1.add_work(&thing);

    // Start worker2
    std::thread other_thread(worker2_thread, &worker2);
    std::this_thread::sleep_for(std::chrono::seconds (1));

    // Start worker1
    worker1.run();

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
//    ret = app_asio_socket(argc, argv);
//    ret = app_im(argc, argv);
//    test_timeout_hash_table();

//    ret = ctx_main();
//    ret = test_reenter();
    ret = test_worker();

    return ret;
}
