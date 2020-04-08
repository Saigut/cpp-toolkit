#include <stdio.h>
#include <json-c/json.h>

#include <iostream>
#include <boost/format.hpp>

int main(int argc, char** argv)
{
    struct json_object * j_obj = json_tokener_parse("{'First': 'Hello', 'Second': 'world'}");
    if (j_obj) {
        printf("%s, %s!\n",
                json_object_get_string(json_object_object_get(j_obj, "First")),
                json_object_get_string(json_object_object_get(j_obj, "Second")));
    }
    std::cout << boost::format("%1% %2% %3% %2% %1% \n") % "11" % "22" % "333";
    return 0;
}
