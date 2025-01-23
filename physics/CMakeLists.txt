
set( sources 
    "api.h"
    "result.h"
    "typedefs.h"
    "particle.h"
    "particle_system.h"
    "particle_system.cpp"
    "emitters.hpp"
    "force_fields.hpp"

    )

motor_vs_src_dir( sources )

add_library( ${cur_lib_name} ${MOTOR_LIBRARY_BUILD_TYPE} ${sources} )
add_library( ${cur_alias_name} ALIAS ${cur_lib_name} ) 

target_link_libraries( ${cur_lib_name}
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


