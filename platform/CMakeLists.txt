
set( sources 
    "api.h"
    "typedefs.h"
    )


set( device_win32_sources 
    "device/win32/rawinput_module.h"
    "device/win32/rawinput_module.cpp"
    "device/win32/xinput_module.h"
    "device/win32/xinput_module.cpp"
)

set( device_linux_sources
  "device/linux/evdev_module.h"
  "device/linux/evdev_module.cpp"
  "device/linux/xlib_module.h"
  "device/linux/xlib_module.cpp"
)

set( unwanted_compiler_options )
set( os_libs )

if( MOTOR_TARGET_OS_WIN )

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
  PUBLIC motor::device
    )

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

