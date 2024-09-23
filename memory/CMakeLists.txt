
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

    "observer.hpp"

    "utility/uint8_msb_stream.hpp"
    "utility/uint8_split_merge.hpp"
    )

motor_vs_src_dir( sources )

add_library( ${cur_lib_name} ${MOTOR_LIBRARY_BUILD_TYPE} ${sources} )
add_library( ${cur_alias_name} ALIAS ${cur_lib_name} )

target_link_libraries( ${cur_lib_name} 
    PUBLIC motor::std
    PUBLIC motor::base 
    )

option( MOTOR_MEMORY_OBSERVER "Memory Observer" False )

if( MOTOR_MEMORY_OBSERVER )
target_compile_definitions( ${cur_lib_name} PUBLIC MOTOR_MEMORY_OBSERVER )
endif()

###########################################################
# SECTION: Build Tree
###########################################################

#motor_export( ${cur_lib_name} )

###########################################################
# SECTION: Install
###########################################################

install_headers( "${sources}" "include/${PROJECT_NAME}/${cur_lib_name}" )
install_library( ${cur_lib_name} ${PROJECT_NAME}-targets )

