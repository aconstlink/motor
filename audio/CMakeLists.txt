
set( sources 
    "api.h"
    "typedefs.h"
    "result.h"
    "enums.h"
    "buffer.hpp"
    "system.h"
    "system.cpp"

    "command_engine.h"
    "command_engine.cpp"

    "object/object.hpp"
    "object/capture_object.hpp"
    "object/buffer_object.hpp"
    "object/synth_object.hpp"

    "ifrontend.h"
    "backend.h"
    "backend.cpp"
    "frontend.hpp"
    )


motor_vs_src_dir( sources )

add_library( ${cur_lib_name} ${MOTOR_LIBRARY_BUILD_TYPE} ${sources} )
add_library( ${cur_alias_name} ALIAS ${cur_lib_name} ) 

target_link_libraries( ${cur_lib_name}
  PUBLIC motor::math
  PUBLIC motor::memory
  PUBLIC motor::log )



###########################################################
# SECTION: Build Tree
###########################################################

##motor_export( ${cur_lib_name} )

###########################################################
# SECTION: Install
###########################################################

install_headers( "${sources}" "include/${PROJECT_NAME}/${cur_lib_name}" )
install_library( ${cur_lib_name} ${PROJECT_NAME}-targets )

