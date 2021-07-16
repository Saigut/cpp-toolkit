include_guard()

function(is_os_linux RET)
    set(${RET} FALSE)
endfunction()

function(target_link_boost_libraries TargetName)
    set(LibPrefix "")
    if(OS_WINDOWS)
        set(LibPrefix "lib")
    endif()
    foreach(arg IN LISTS ARGN)
        target_link_libraries(${TargetName} ${LibPrefix}${arg})
    endforeach()
endfunction()

function(libp2p_install targets)
endfunction()

function(libp2p_add_library target)
    add_library(${target}
        ${ARGN}
        )
#    libp2p_install(${target})
endfunction()

function(add_proto_library NAME)
endfunction()
