#include <program_main.h>

#include <stdio.h>
#include <json-c/json.h>

#include <iostream>
#include <boost/format.hpp>

#include <mod_socket/mod_socket.h>
#include <app_socket/app_socket.h>
#include <app_chat/app_chat.h>
#include <app_asio_socket/app_asio_socket.h>


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
