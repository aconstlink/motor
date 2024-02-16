
set( sources 
    "api.h"
    "result.h"
    "typedefs.h"

    "camera/pinhole_camera.h"
    "camera/generic_camera.h"
    "camera/generic_camera.cpp"

    "font/text_render_2d.h"
    "font/text_render_2d.cpp"

    #"primitive/line_render_2d.h"
    #"primitive/line_render_2d.cpp"
    #"primitive/line_render_3d.h"
    #"primitive/line_render_3d.cpp"
    #"primitive/tri_render_2d.h"
    #"primitive/tri_render_2d.cpp"
    #"primitive/primitive_render_2d.h"
    #"primitive/primitive_render_2d.cpp"
    #"primitive/tri_render_3d.h"
    #"primitive/tri_render_3d.cpp"
    #"primitive/primitive_render_3d.h"
    #"primitive/primitive_render_3d.cpp"
    
    #"sprite/sprite_render_2d.h"
    #"sprite/sprite_render_2d.cpp"

    "util/quad.h"
    "util/quad.cpp"
    )

motor_vs_src_dir( sources )

add_library( ${cur_lib_name} ${MOTOR_LIBRARY_BUILD_TYPE} ${sources} )
add_library( ${cur_alias_name} ALIAS ${cur_lib_name} ) 

target_link_libraries( ${cur_lib_name}
  PUBLIC motor::font
  PUBLIC motor::device
  PUBLIC motor::graphics
  PUBLIC motor::memory
  PUBLIC motor::log 
    )

###########################################################
# SECTION: Build Tree
###########################################################

##motor_export( ${cur_lib_name} )

###########################################################
# SECTION: Install
###########################################################

install_headers( "${sources}" "include/${PROJECT_NAME}/${cur_lib_name}" )
install_library( ${cur_lib_name} ${PROJECT_NAME}-targets )

