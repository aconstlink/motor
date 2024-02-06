
set( sources
    "api.h"
    "typedefs.h"

    "allocator.hpp"
    "global.h"
    "global.cpp"

    "manager/manager.h"
    "manager/manager.cpp"
    
    "malloc_guard.hpp"
    
    "arena.hpp"
    )

motor_vs_src_dir( sources )

add_library( ${cur_lib_name} ${MOTOR_LIBRARY_BUILD_TYPE} ${sources} )
add_library( ${cur_alias_name} ALIAS ${cur_lib_name} )

target_link_libraries( ${cur_lib_name} 
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

