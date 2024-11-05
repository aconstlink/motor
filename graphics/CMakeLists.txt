
set( sources 
    "api.h"
    "result.h"
    "protos.h"
    "typedefs.h"
    "types.h"
    "object.hpp"

    "render_engine.h"
    "render_engine.cpp"

    "state/render_states.h"
    "state/state_set.h"

    "backend/bid_generator.h"
    "backend/bid_generator.cpp"
    "backend/gen4/types.h"
    "backend/gen4/backend.h"
    "backend/gen4/null.h"
    "backend/gen4/null.cpp"

    "frontend/ifrontend.h"
    "frontend/gen4/frontend.hpp"

    "shader/shader_set.hpp"
    "shader/shader.hpp"
    "shader/binding_point.h"
    "shader/msl_bridge.hpp"
    "shader/shader_bindings.hpp"
    "shader/compilation_listener.h"

    "buffer/vertex_attribute.h"
    "buffer/vertex_buffer.hpp"
    "buffer/index_buffer.hpp"
    "buffer/data_buffer.hpp"

    "object/render_object.h"
    "object/geometry_object.h"
    "object/image_object.h"
    "object/msl_object.h"
    "object/msl_object.cpp"
    "object/shader_object.h"
    "object/framebuffer_object.h"
    "object/state_object.h"
    "object/array_object.h"
    "object/streamout_object.h"

    "variable/variables.hpp"
    "variable/variable_set.hpp"
    "variable/type_traits.hpp"
    "variable/wire_variable_bridge.h"
    "variable/wire_variable_bridge.cpp"

    "texture/image.hpp"
    )



motor_vs_src_dir( sources )

add_library( ${cur_lib_name} ${MOTOR_LIBRARY_BUILD_TYPE} ${sources} )
add_library( ${cur_alias_name} ALIAS ${cur_lib_name} ) 

target_link_libraries( ${cur_lib_name} 
  PUBLIC motor::memory
  PUBLIC motor::log
  PUBLIC motor::std
  PUBLIC motor::concurrent
  PUBLIC motor::msl
  PUBLIC motor::wire )

###########################################################
# SECTION: Build Tree
###########################################################

##motor_export( ${cur_lib_name} )

###########################################################
# SECTION: Install
###########################################################

install_headers( "${sources}" "include/${PROJECT_NAME}/${cur_lib_name}" )
install_library( ${cur_lib_name} ${PROJECT_NAME}-targets )

