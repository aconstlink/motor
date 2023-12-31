
set( sources 
    "api.h"
    "typedefs.h"
    "result.h"
    
    "global.h"
    "global.cpp"
    )

set( unwanted_compiler_options )
set( os_libs )

##
## Device Section
##

if( MOTOR_TARGET_OS_WIN )

    set( device_win32_sources 
      "device/win32/rawinput_module.h"
      "device/win32/rawinput_module.cpp"
      "device/win32/xinput_module.h"
      "device/win32/xinput_module.cpp" )

    ###################################################
    # device
    list( APPEND sources ${device_win32_sources} )
    set( os_libs winmm.lib xinput )

    if( MOTOR_COMPILER_MSC )
        # warning C4005 : '__useHeader' : macro redefinition 
        # Note: C4005 seem to be a redefinition bug in the window header files.
        set( unwanted_compiler_options "/wd4005" )
    endif()

    ###################################################
    # other lib

elseif( MOTOR_TARGET_OS_LIN )
    set( device_linux_sources
      "device/linux/evdev_module.h"
      "device/linux/evdev_module.cpp"
      "device/linux/xlib_module.h"
      "device/linux/xlib_module.cpp")
  list( APPEND sources ${device_linux_sources} )
endif()

##
## Graphics Section
##

if( MOTOR_GRAPHICS_OPENGL )
list( APPEND sources 
    "graphics/gl/gl4.h"
    "graphics/gl/gl4.cpp"
    "graphics/gl/gl4_convert.h"
    "graphics/gl/gl_context.h"
)
endif()
if( MOTOR_GRAPHICS_OPENGLES )
list( APPEND sources_ 
    "graphics/gl/es3.h"
    "graphics/gl/es3.cpp"
    "graphics/gl/es_context.h"
)
endif()
if( MOTOR_GRAPHICS_DIRECT3D )
list( APPEND sources 
    "graphics/d3d/d3d11.h"
    "graphics/d3d/d3d11.cpp"
    "graphics/d3d/d3d11_context.h"
    "graphics/d3d/d3d11_convert.h"
)
endif()

##
## Application Section
##

if( MOTOR_GRAPHICS_WGL )

    list( APPEND sources
      "application/wgl/wgl_context.h"
      "application/wgl/wgl_context.cpp" )
endif()

if( MOTOR_GRAPHICS_DIRECT3D )

    list( APPEND sources
      "application/d3d/dx11_context.h"
      "application/d3d/dx11_context.cpp" )
endif()



if( MOTOR_GRAPHICS_GLX )

    list( APPEND sources
      #"platform/glx/glx_window.h"
      #"platform/glx/glx_window.cpp"
      "application/glx/glx_context.h"
      "application/glx/glx_context.cpp"
      "application/glx/glx_context_gl.cpp" )
endif()

if( MOTOR_GRAPHICS_EGL )

    list( APPEND sources_
      "platform/egl/egl_window.h"
      "platform/egl/egl_window.cpp"
      "platform/egl/egl_context.h"
      "platform/egl/egl_context.cpp" )
endif()

if( MOTOR_WINDOW_SYSTEM_WIN32 )

    list( APPEND sources
      "application/win32/win32_carrier.h"
      "application/win32/win32_carrier.cpp" )
    add_definitions( -DWIN32_LEAN_AND_MEAN )

elseif( MOTOR_WINDOW_SYSTEM_XLIB )

    list( APPEND sources
    "application/xlib/xlib_carrier.h"
    "application/xlib/xlib_carrier.cpp"
    
     )
    set( os_libs ${X11_LIBRARIES} )

else()
    message( FATAL_ERROR "Window System: No Os specified." )
endif()


motor_vs_src_dir( sources )

add_library( ${cur_lib_name} ${MOTOR_LIBRARY_BUILD_TYPE} ${sources} )
add_library( ${cur_alias_name} ALIAS ${cur_lib_name} ) 
target_compile_options( ${cur_lib_name} PRIVATE ${unwanted_compiler_options} )
target_compile_options( ${cur_lib_name} PUBLIC ${MOTOR_WINDOW_SYSTEM_DEFINES} )

target_link_libraries( ${cur_lib_name}
  PRIVATE ${os_libs}
  PUBLIC motor::std 
  PUBLIC motor::memory
  PUBLIC motor::log 
  PUBLIC motor::device
  PUBLIC motor::graphics
  PUBLIC motor::application

  # have changed
  PUBLIC ct_motor_graphics_all
    )

if( TARGET motor::ogl )
    target_link_libraries( ${cur_lib_name} 
      PUBLIC gl_headers
      PUBLIC motor::ogl )
endif()

# @todo need to check this out once more
# + need some api for midi controls like 
# back in the days
if( TARGET rtmidi )
  target_link_libraries( ${cur_lib_name}
    PUBLIC rtmidi )
endif()

if( MOTOR_COMPILER_GNU )
  target_compile_options( ${cur_lib_name} PRIVATE -Wno-deprecated-declarations )
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

