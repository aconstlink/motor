
set( sources
    "api.h"
    "typedefs.h"
    "result.h"
    "protos.h"
    "global.h"
    "global.cpp"
    "ivisitable.hpp"
    "macros.h"

    "node/group.h"
    "node/group.cpp"
    "node/leaf.h"
    "node/leaf.cpp"
    "node/node.h"
    "node/node.cpp"
    "node/logic_group.h"
    "node/logic_group.cpp"
    "node/switch_group.h"
    "node/switch_group.cpp"
    "node/logic_leaf.h"
    "node/logic_leaf.cpp"

    "visitor/ivisitor.h"
    "visitor/log_visitor.h"
    "visitor/log_visitor.cpp"
    "visitor/code_exe_visitor.h"
    "visitor/code_exe_visitor.cpp"
    
    "visitor/trafo_visitor.h"
    "visitor/trafo_visitor.cpp"
    "visitor/variable_update_visitor.h"
    "visitor/variable_update_visitor.cpp"
    "visitor/default_visitor.h"
    "visitor/default_visitor.cpp"
    "visitor/search_by_name_visitor.hpp"
    
    "visitor/graphics/render_visitor.h"
    "visitor/graphics/render_visitor.cpp"
    "visitor/graphics/init_msl_visitor.hpp"
    "visitor/graphics/release_msl_visitor.hpp"
    "visitor/graphics/add_shader_to_set_visitor.hpp"
    


    "component/icomponent.h"
    "component/icontroller.h"
    "component/code_component.h"
    "component/code_component.cpp"
    "component/name_component.hpp"
    "component/graphics/msl_component.h"
    "component/graphics/msl_component.cpp"
    "component/graphics/render_settings_component.hpp"
    "component/trafo3d_component.h"
    "component/trafo3d_component.cpp"
    "component/camera_component.h"
    "component/camera_component.cpp"
    "component/graphics/render_variables_component.hpp"
    "component/variables_component.hpp"
    "component/graphics/config_graphics_component.h"
    "component/graphics/config_graphics_component.cpp"
    "component/graphics/shader_change_observer_component.h"
    "component/graphics/shader_change_observer_component.cpp"
    "component/graphics/msl_set_component.hpp"

    "component/animation/animation_component.h"
    "component/animation/animation_component.cpp"
    "component/animation/animation_controller.h"
    "component/animation/animation_controller.cpp"
    "component/animation/animation_track.hpp"
    )

motor_vs_src_dir( sources )

add_library( ${cur_lib_name} ${MOTOR_LIBRARY_BUILD_TYPE} ${sources} )
add_library( ${cur_alias_name} ALIAS ${cur_lib_name} ) 

target_link_libraries( ${cur_lib_name}
    PUBLIC motor::gfx
    PUBLIC motor::graphics
    PUBLIC motor::wire
    PUBLIC motor::core
    )

# copy and paste mistake here?
#option( MOTOR_PROFILING "motor profile timing" False )

#if( MOTOR_PROFILING )
#target_compile_definitions( ${cur_lib_name} PUBLIC MOTOR_PROFILING )
#endif()

###########################################################
# SECTION: Build Tree
###########################################################

##motor_export( ${cur_lib_name} )

###########################################################
# SECTION: Install
###########################################################

install_headers( "${sources}" "include/${PROJECT_NAME}/${cur_lib_name}" )
install_library( ${cur_lib_name} ${PROJECT_NAME}-targets )

