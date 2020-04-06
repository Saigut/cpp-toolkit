#include <stdio.h>
#include <json-c/json.h>

int main(int argc, char** argv)
{
    struct json_object * j_obj = json_tokener_parse("{'First': 'Hello', 'Second': 'world'}");
    if (j_obj) {
        printf("%s, %s!\n",
                json_object_get_string(json_object_object_get(j_obj, "First")),
                json_object_get_string(json_object_object_get(j_obj, "Second")));
    }
    return 0;
}
