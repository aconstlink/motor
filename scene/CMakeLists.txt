
set( sources
    "api.h"
    "typedefs.h"
    "result.h"
    "protos.h"

    "node/group.h"
    "node/group.cpp"
    "node/leaf.h"
    "node/leaf.cpp"
    "node/node.h"
    "node/node.cpp"
    "node/group/logic_group.h"
    "node/group/logic_group.cpp"
    "node/group/switch_group.h"
    "node/group/switch_group.cpp"
    "node/group/trafo3d_node.h"
    "node/group/trafo3d_node.cpp"
    "node/leaf/logic_leaf.h"
    "node/leaf/logic_leaf.cpp"
    "node/leaf/camera_node.h"
    "node/leaf/camera_node.cpp"

    "visitor/ivisitor.h"
    "visitor/visitor.h"
    "visitor/log_visitor.h"
    "visitor/log_visitor.cpp"
    "visitor/code_exe_visitor.h"
    "visitor/code_exe_visitor.cpp"
    "visitor/render_visitor.h"
    "visitor/render_visitor.cpp"
    "visitor/trafo_visitor.h"
    "visitor/trafo_visitor.cpp"

    "component/icomponent.h"
    "component/code_component.h"
    "component/code_component.cpp"
    "component/name_component.hpp"

    "component/msl_component.h"
    "component/msl_component.cpp"
    "component/render_state_component.h"
    "component/render_state_component.cpp"
    )

motor_vs_src_dir( sources )

add_library( ${cur_lib_name} ${MOTOR_LIBRARY_BUILD_TYPE} ${sources} )
add_library( ${cur_alias_name} ALIAS ${cur_lib_name} ) 

target_link_libraries( ${cur_lib_name}
    PUBLIC motor::gfx
    PUBLIC motor::graphics
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

