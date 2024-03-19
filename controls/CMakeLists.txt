
set( sources 
    "api.h"
    "result.h"
    "typedefs.h"

    "device.hpp"
    "component.hpp"
    "imapping.hpp"
    "mapping.hpp"
    "system.h"
    "system.cpp"
    "iobserver.hpp"

    "imodule.h"
    "modules/vdev_module.h"
    "modules/vdev_module.cpp"

    "layouts/ascii_keyboard.hpp"
    "layouts/three_mouse.hpp"
    "layouts/ps3_controller.hpp"
    "layouts/xbox_controller.hpp"
    "layouts/game_controller.hpp"
    "layouts/midi_controller.hpp"

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

    "midi/device_demuxer.hpp"
    "midi/korg_demuxer.hpp"
    "midi/akai_demuxer.hpp"
    "midi/behringer_demuxer.hpp"
    "midi/midi_message.hpp"
    "midi/midi_observer.hpp"
    )

motor_vs_src_dir( sources )

add_library( ${cur_lib_name} ${MOTOR_LIBRARY_BUILD_TYPE} ${sources} )
add_library( ${cur_alias_name} ALIAS ${cur_lib_name} ) 
target_compile_options( ${cur_lib_name} PRIVATE ${unwanted_compiler_options} )
target_compile_options( ${cur_lib_name} PUBLIC ${MOTOR_WINDOW_SYSTEM_DEFINES} )

target_link_libraries( ${cur_lib_name}
  PUBLIC motor::std 
  PUBLIC motor::memory
  PUBLIC motor::log 
  )

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

