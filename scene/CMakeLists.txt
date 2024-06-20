
set( sources
    "api.h"
    "typedefs.h"
    "result.h"
    "protos.h"

    "node/decorator.h"
    "node/decorator.cpp"
    "node/group.h"
    "node/group.cpp"
    "node/leaf.h"
    "node/leaf.cpp"
    "node/node.h"
    "node/node.cpp"
    "node/decorator/logic_decorator.h"
    "node/decorator/logic_decorator.cpp"
    "node/group/logic_group.h"
    "node/group/logic_group.cpp"
    "node/group/switch_group.h"
    "node/group/switch_group.cpp"
    "node/leaf/logic_leaf.h"
    "node/leaf/logic_leaf.cpp"

    "visitor/ivisitor.h"
    )

motor_vs_src_dir( sources )

add_library( ${cur_lib_name} ${MOTOR_LIBRARY_BUILD_TYPE} ${sources} )
add_library( ${cur_alias_name} ALIAS ${cur_lib_name} ) 

target_link_libraries( ${cur_lib_name}
    PUBLIC motor::std
    PUBLIC motor::memory
    PUBLIC motor::log 
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

