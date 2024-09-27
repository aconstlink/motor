
set( sources
    "api.h"
    "typedefs.h"
    "result.h"
    "protos.h"
    "global.h"
    "global.cpp"
    "double_dispatch.h"
    "ivisitable.hpp"

    "node/decorator.h"
    "node/decorator.cpp"
    "node/group.h"
    "node/group.cpp"
    "node/leaf.h"
    "node/leaf.cpp"
    "node/node.h"
    "node/node.cpp"
    "node/logic_decorator.h"
    "node/logic_decorator.cpp"
    "node/logic_group.h"
    "node/logic_group.cpp"
    "node/switch_group.h"
    "node/switch_group.cpp"
    "node/trafo3d_node.h"
    "node/trafo3d_node.cpp"
    "node/logic_leaf.h"
    "node/logic_leaf.cpp"
    "node/camera_node.h"
    "node/camera_node.cpp"

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

