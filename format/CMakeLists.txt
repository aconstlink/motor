
set( sources 
    "api.h"
    "typedefs.h"

    "future_item.hpp"
    "future_items.hpp"
    "global.h"
    "global.cpp"
    "module_factory.hpp"
    "module_registry.hpp"

    "imodule.h"
    
    "obj/obj_module_structs.h"
    "obj/obj_module.cpp"
    "obj/obj_module_mtl.cpp"
    "obj/obj_module.h"

    "gltf/gltf_module_structs.h"
    "gltf/cgltf_module.h"
    "gltf/cgltf_module.cpp"

    "stb/stb_module.cpp"
    "stb/stb_module.h"

    "wav/wav_module.cpp"
    "wav/wav_module.h"

    "msl/msl_module.h"
    "msl/msl_module.cpp"

    "motor/motor_structs.h"
    "motor/motor_module.h"
    "motor/motor_module.cpp"

    "aseprite/aseprite_structs.h"
    "aseprite/aseprite_module.h"
    "aseprite/aseprite_module.cpp"
    )

motor_vs_src_dir( sources )

add_library( ${cur_lib_name} ${MOTOR_LIBRARY_BUILD_TYPE} ${sources} )
add_library( ${cur_alias_name} ALIAS ${cur_lib_name} ) 

target_link_libraries( ${cur_lib_name}
  PRIVATE stb
  PRIVATE rapidxml
  PRIVATE nlohmann_json
  PRIVATE cgltf
  PUBLIC motor::geometry
  PUBLIC motor::font
  PUBLIC motor::msl
  PUBLIC motor::audio
  PUBLIC motor::graphics
  PUBLIC motor::io
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

