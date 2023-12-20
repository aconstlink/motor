
set( sources 
    "api.h"
    "result.h"
    "protos.h"
    "typedefs.h"
    "types.h"
    "object.hpp"

    "id.hpp"

    "async.h"
    "async.cpp"
    "async_id.hpp"

    "state/render_states.h"
    "state/state_set.h"

    "backend/types.h"
    "backend/backend.h"
    "backend/backend.cpp"
    "backend/null/null.h"
    "backend/null/null.cpp"

    "shader/shader_set.hpp"
    "shader/shader.hpp"
    "shader/variable_binding.h"
    "shader/nsl_bridge.hpp"

    "buffer/vertex_attribute.h"
    "buffer/vertex_buffer.hpp"
    "buffer/index_buffer.hpp"
    "buffer/data_buffer.hpp"

    "object/render_object.h"
    "object/geometry_object.h"
    "object/image_object.h"
    "object/shader_object.h"
    "object/framebuffer_object.h"
    "object/state_object.h"
    "object/array_object.h"
    "object/streamout_object.h"

    "variable/variables.hpp"
    "variable/variable_set.hpp"
    "variable/type_traits.hpp"

    "texture/image.hpp"
    )



motor_vs_src_dir( sources )

add_library( ${cur_lib_name} ${MOTOR_LIBRARY_BUILD_TYPE} ${sources} )
add_library( ${cur_alias_name} ALIAS ${cur_lib_name} ) 

target_link_libraries( ${cur_lib_name} 
  PUBLIC motor::memory
  PUBLIC motor::log
  PUBLIC motor::std
  PUBLIC motor::concurrent )

###########################################################
# SECTION: Build Tree
###########################################################

##motor_export( ${cur_lib_name} )

###########################################################
# SECTION: Install
###########################################################

install_headers( "${sources}" "include/${PROJECT_NAME}/${cur_lib_name}" )
install_library( ${cur_lib_name} ${PROJECT_NAME}-targets )

