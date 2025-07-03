
set( gl_sources
    gl/gl.h
    gl/gl.cpp
    gl/gl_load.h
    gl/gl_load.cpp
    gl/error.hpp
    gl/convert.hpp
)

set( es_sources
    es/es.h
    es/es.cpp
    #es/error.hpp
    #es/convert.hpp
)

set( wgl_sources
    wgl/wgl.h
    wgl/wgl.cpp
)

set( glx_sources
    glx/glx.h
    glx/glx.cpp
)

set( egl_sources
    egl/egl.h
    egl/egl.cpp
)

set( sources 
    api.h
    result.h
    typedefs.h
    protos.h

    ${gl_sources}
)

if( MOTOR_TARGET_OS_WIN )
    set( sources ${sources} ${wgl_sources} )
    add_definitions( -DWIN32_LEAN_AND_MEAN )
elseif( MOTOR_TARGET_OS_LIN )
    set( sources ${sources} ${glx_sources} )
endif()

motor_vs_src_dir( sources )

add_library( ${cur_lib_name} ${MOTOR_LIBRARY_BUILD_TYPE} ${sources} )
add_library( ${cur_alias_name} ALIAS ${cur_lib_name} )

target_link_libraries( ${cur_lib_name}
  PRIVATE ct_motor_graphics_gl
  PRIVATE ct_motor_compiler_options
  PRIVATE OpenGL::GL 
  PRIVATE gl_headers
  PUBLIC motor::core
)

###########################################################
# SECTION: Build Tree
###########################################################

#so_export( ${cur_lib_name} )

###########################################################
# SECTION: Install
###########################################################

install_headers( "${sources}" "include/${PROJECT_NAME}/${cur_lib_name}" )
install_library( ${cur_lib_name} ${PROJECT_NAME}-targets )
