
set( sources 
    "api.h"
    "typedefs.h"
    "result.h"
    
    "global.h"
    "global.cpp"

    "cpu_id.h"
    "cpu_id.cpp"

    "graphics/datas.hpp"
    )

set( unwanted_compiler_options )
set( os_libs )

##
## Controls Section
##

if( MOTOR_TARGET_OS_WIN )

    set( controls_win32_sources 
        "controls/win32/rawinput_module.h"
        "controls/win32/rawinput_module.cpp"
        "controls/win32/xinput_module.h"
        "controls/win32/xinput_module.cpp"
        "controls/win32/midi_module.h"
        "controls/win32/midi_module.cpp")

    ###################################################
    # controls
    list( APPEND sources ${controls_win32_sources} )
    list( APPEND os_libs winmm xinput )

    if( MOTOR_COMPILER_MSC )
        # warning C4005 : '__useHeader' : macro redefinition 
        # Note: C4005 seem to be a redefinition bug in the window header files.
        set( unwanted_compiler_options "/wd4005" )
    endif()

    ###################################################
    # other lib

elseif( MOTOR_TARGET_OS_LIN )
    set( controls_linux_sources
      "controls/linux/evdev_module.h"
      "controls/linux/evdev_module.cpp"
      "controls/linux/xlib_module.h"
      "controls/linux/xlib_module.cpp")
  list( APPEND sources ${controls_linux_sources} )
endif()

##
## Network Section
##

if( MOTOR_TARGET_OS_WIN )
  set( network_win32_sources
    "network/win32/win32_net_module.h"
    "network/win32/win32_net_module.cpp"
    )

  list( APPEND sources ${network_win32_sources} )
  list( APPEND os_libs winmm Ws2_32 )

elseif( MOTOR_TARGET_OS_LIN )
  set( network_unix_sources
  "network/unix/unix_net_module.h"
  "network/unix/unix_net_module.cpp"
  )

  list( APPEND sources ${network_unix_sources} )
  #list( APPEND os_libs *** )
endif()

list( APPEND sources "network/network_module_creator.hpp" )

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

if( MOTOR_GRAPHICS_GLX AND TARGET OpenGL::GL )

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
    list( APPEND os_libs ${X11_LIBRARIES} )

else()
    message( FATAL_ERROR "Window System: No Os specified." )
endif()

##
## Audio Section
##

list( APPEND sources
  "audio/audio_capture_helper.h"
  "audio/audio_capture_helper.cpp"
  # make oal optional
  "audio/oal/oal.h"
  "audio/oal/oal.cpp"
     )
if( MOTOR_TARGET_OS_WIN )
  list( APPEND sources
    "audio/wasapi/wasapi_capture_helper.h"
    "audio/wasapi/wasapi_capture_helper.cpp"
    #"audio/xaudio/xaudio2.h"
    #"audio/xaudio/xaudio2.cpp"
  )
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
  PUBLIC motor::controls
  PUBLIC motor::graphics
  PUBLIC motor::application
  PUBLIC motor::audio
  PUBLIC motor::profiling
  PUBLIC motor::network

  # make oal optional
  PRIVATE OpenAL

  # have changed
  PRIVATE ct_motor_graphics_all
    )

if( TARGET motor::ogl )
    target_link_libraries( ${cur_lib_name} 
      PRIVATE gl_headers
      PRIVATE motor::ogl )
endif()

# @todo need to check this out once more
# + need some api for midi controls like 
# back in the days
if( TARGET rtmidi )
  target_link_libraries( ${cur_lib_name}
    PUBLIC rtmidi )
endif()

if( XAUDIO2_ENABLE )
  target_link_libraries( ${cur_lib_name}
    PRIVATE xaudio2 )
endif()

if( MOTOR_TARGET_ARCH_BIG_ENDIAN )
  target_compile_options( ${cur_lib_name} PRIVATE -DMOTOR_BIG_ENDIAN )
endif()

if( MOTOR_COMPILER_GNU )
  target_compile_options( ${cur_lib_name} PUBLIC -Wno-pessimizing-move )
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

