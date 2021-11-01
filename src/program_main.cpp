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
#include <app_worker/app_worker.h>
#include <mod_worker/mod_worker.h>
#include <mod_tcp_hole_punching/mod_tcp_hole_punching.h>

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

int test_lockfree(int argc, char** argv)
{
    boost::lockfree::queue<WorkWrap*, boost::lockfree::capacity<5001>> lf_q;

    WorkWrap* work_wrap = new WorkWrap(std::make_shared<Work>());
    expect_ret_val(lf_q.push(work_wrap), -1);

    WorkWrap* work_wrap2;
    expect_ret_val(lf_q.pop(work_wrap2), -1);

    delete work_wrap2;

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
//    ret = app_worker(argc, argv);
//    ret = test_lockfree(argc, argv);
    ret = mod_tcp_hole_test(argc, argv);

    return ret;
}
