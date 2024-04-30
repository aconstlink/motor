

set( SOURCES ${SOURCES}
    "api.h"
    "typedefs.h"

    "imodule.h"
    "system.h"
    "system.cpp"
    "ipv4_address.hpp"

    "convert.hpp"

    "socket/connection_info.hpp"
    "socket/tcp_socket.h"
    "socket/tcp_socket.cpp"
    )

motor_vs_src_dir( SOURCES )

add_library( ${cur_lib_name} ${MOTOR_LIBRARY_BUILD_TYPE} ${SOURCES} )
add_library( ${cur_alias_name} ALIAS ${cur_lib_name} )


target_link_libraries( ${cur_lib_name}
PUBLIC motor::concurrent
PUBLIC motor::log
PUBLIC motor::memory
PUBLIC motor::std
PUBLIC motor::core
)

###########################################################
# SECTION: Build Tree
###########################################################

#motor_export( ${cur_lib_name} )

###########################################################
# SECTION: Install
###########################################################

install_headers( "${sources}" "include/${PROJECT_NAME}/${cur_lib_name}" )
install_library( ${cur_lib_name} ${PROJECT_NAME}-targets )
