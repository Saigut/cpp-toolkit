
set(V_program_name main)
add_executable(${V_program_name} exec_main.cpp)
set(V_dep_libs ${CONAN_LIBS})
list(FILTER V_dep_libs EXCLUDE REGEX "boost")
target_link_libraries(${V_program_name}
    cpp_toolkit
    ${GV_gen_grpc_cpp_lib_bname}
    ${V_dep_libs})
target_link_libraries(${V_program_name} boost::boost ${PLATFORM_LINK_LIB})
