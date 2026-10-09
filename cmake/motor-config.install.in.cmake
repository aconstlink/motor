@PACKAGE_INIT@

include( CMakeFindDependencyMacro )

if( NOT @MOTOR_LIBRARY_BUILD_SHARED@ )
    find_dependency( OpenAL CONFIG HINTS "${PACKAGE_PREFIX_DIR}" )
endif()
if( @MOTOR_GRAPHICS_OPENGL@ )
    find_dependency( OpenGL )
endif()
if( @MOTOR_WINDOW_SYSTEM_XLIB@ AND NOT @MOTOR_LIBRARY_BUILD_SHARED@ )
    find_dependency( X11 )
endif()

include( "${CMAKE_CURRENT_LIST_DIR}/motor-targets.cmake" )
set( motor_VERSION "@MOTOR_VERSION@" )

foreach( component IN LISTS motor_FIND_COMPONENTS )
    if( TARGET motor::${component} )
        set( motor_${component}_FOUND TRUE )
    else()
        set( motor_${component}_FOUND FALSE )
    endif()
endforeach()
check_required_components( motor )
