
set( sources 
    "api.h"
    "result.h"
    "protos.h"
    "typedefs.h"
    "structs.h"

    "imgui/imgui.h"
    "imgui/imgui.cpp"
    "imgui/sprite_editor.h"
    "imgui/sprite_editor.cpp"
    "imgui/custom_widgets.h"
    "imgui/custom_widgets.cpp"
    "imgui/imgui_custom.h"
    "imgui/imgui_custom.cpp"

    "imgui/timeline.h"
    "imgui/timeline.cpp"
    "imgui/keyframe_editor.h"
    "imgui/keyframe_editor.cpp"
    "imgui/player_controller.h"
    "imgui/player_controller.cpp"
    "imgui/engine_profiling.h"
    "imgui/engine_profiling.cpp"

    "imgui/imnodes_wire.h"
    "imgui/imnodes_wire.cpp"
    "imgui/imgui_property.h"
    "imgui/imgui_property.cpp"

    "imgui/node_kit/imgui_node_visitor.h"
    "imgui/node_kit/imgui_node_visitor.cpp"
    "imgui/node_kit/imgui_node_component.h"
    "imgui/node_kit/imgui_node_component.cpp"
    )

motor_vs_src_dir( sources )

add_library( ${cur_lib_name} ${MOTOR_LIBRARY_BUILD_TYPE} ${sources} )
add_library( ${cur_alias_name} ALIAS ${cur_lib_name} ) 

target_link_libraries( ${cur_lib_name}
  PRIVATE stb
  PUBLIC imgui
  PUBLIC implot
  PUBLIC imnodes
  PUBLIC motor::io
  PUBLIC motor::controls
  PUBLIC motor::graphics
  PUBLIC motor::memory
  PUBLIC motor::log
  PUBLIC motor::profiling
  PUBLIC motor::property
  PUBLIC motor::gfx
  PUBLIC motor::format
  PUBLIC motor::scene
  PUBLIC motor::wire
    )

if( MOTOR_COMPILER_GNU )
  target_compile_options( ${cur_lib_name} PUBLIC -Wno-pessimizing-move )
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
