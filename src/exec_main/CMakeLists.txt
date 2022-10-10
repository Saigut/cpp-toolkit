
set(V_program_name main)
add_executable(${V_program_name} exec_main.cpp)
target_link_libraries(${V_program_name}
    cpp_toolkit
    ${GV_gen_grpc_cpp_lib_bname}
    ${CONAN_LIBS}
    ${PLATFORM_LINK_LIB})
