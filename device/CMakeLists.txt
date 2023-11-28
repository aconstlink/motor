
set( sources 
    "api.h"
    "result.h"
    "typedefs.h"
    "global.h"
    "global.cpp"

    "device.hpp"
    "component.hpp"
    "imapping.hpp"
    "mapping.hpp"
    "system.h"
    "system.cpp"
    
    "imodule.h"
    "modules/vdev_module.h"
    "modules/vdev_module.cpp"

    "layouts/ascii_keyboard.hpp"
    "layouts/three_mouse.hpp"
    "layouts/ps3_controller.hpp"
    "layouts/xbox_controller.hpp"
    "layouts/game_controller.hpp"

    "components/button.hpp"
    "components/key.hpp"
    "components/knob.hpp"
    "components/led.hpp"
    "components/light.hpp"
    "components/vibrator.hpp"
    "components/point.hpp"
    "components/scroll.hpp"
    "components/slider.hpp"
    "components/stick.hpp"
    "components/touch.hpp"
    )


set( win32_sources 
    "modules/win32/rawinput_module.h"
    "modules/win32/rawinput_module.cpp"
    "modules/win32/xinput_module.h"
    "modules/win32/xinput_module.cpp"
)

set( linux_sources
  "modules/linux/evdev_module.h"
  "modules/linux/evdev_module.cpp"
  "modules/linux/xlib_module.h"
  "modules/linux/xlib_module.cpp"
)

set( unwanted_compiler_options )
set( os_libs )

if( MOTOR_TARGET_OS_WIN )
  list( APPEND sources ${win32_sources} )
  set( os_libs winmm.lib xinput )

  if( MOTOR_COMPILER_MSC )
        # warning C4005 : '__useHeader' : macro redefinition 
        # Note: C4005 seem to be a redefinition bug in the window header files.
        set( unwanted_compiler_options "/wd4005" )
    endif()
endif()

if( MOTOR_WINDOW_SYSTEM_XLIB )
set( os_libs ${X11_LIBRARIES} )
endif()

if( MOTOR_TARGET_OS_LIN )
  list( APPEND sources ${linux_sources} )
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
    )

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

