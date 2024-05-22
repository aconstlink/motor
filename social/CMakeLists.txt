

set( SOURCES ${SOURCES}
    "api.h"
    "typedefs.h"

    "twitch/irc_parser.hpp"
    "twitch/twitch_irc_client.h"
    "twitch/twitch_irc_client.cpp"
    )

motor_vs_src_dir( SOURCES )

add_library( ${cur_lib_name} ${MOTOR_LIBRARY_BUILD_TYPE} ${SOURCES} )
add_library( ${cur_alias_name} ALIAS ${cur_lib_name} )


target_link_libraries( ${cur_lib_name}
PUBLIC motor::network
PUBLIC motor::io
PUBLIC motor::concurrent
PUBLIC motor::log
PUBLIC motor::memory
PUBLIC motor::std
PUBLIC motor::core
PRIVATE nlohmann_json
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
