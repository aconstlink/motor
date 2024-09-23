
set( sources
    "api.h"
    "typedefs.h"

    "global.h"
    "global.cpp"
    "manager.h"
    "manager.cpp"
    "push_guard.hpp"
    "probe_guard.hpp"
    )

motor_vs_src_dir( sources )

add_library( ${cur_lib_name} ${MOTOR_LIBRARY_BUILD_TYPE} ${sources} )
add_library( ${cur_alias_name} ALIAS ${cur_lib_name} ) 

target_link_libraries( ${cur_lib_name}
    PUBLIC motor::core
    )

option( MOTOR_PROFILING "motor profile timing" False )

if( MOTOR_PROFILING )
target_compile_definitions( ${cur_lib_name} PUBLIC MOTOR_PROFILING )
endif()

###########################################################
# SECTION: Build Tree
###########################################################

##motor_export( ${cur_lib_name} )

###########################################################
# SECTION: Install
###########################################################

install_headers( "${sources}" "include/${PROJECT_NAME}/${cur_lib_name}" )
install_library( ${cur_lib_name} ${PROJECT_NAME}-targets )

