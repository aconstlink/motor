
set( MOTOR_WINDOW_SYSTEM_CONFIGURED FALSE )

if( NOT MOTOR_OS_CONFIGURED )
    message( FATAL_ERROR "Configured Target OS required." )
endif()

set( MOTOR_WINDOW_SYSTEM_NULL ON )
set( MOTOR_WINDOW_SYSTEM_WIN32 OFF )
set( MOTOR_WINDOW_SYSTEM_XLIB OFF )
set( MOTOR_WINDOW_SYSTEM_WAYLAND OFF )

if( MOTOR_TARGET_OS_WIN )
    
    set( MOTOR_TARGET_WINDOW_SYSTEM_API "win32" CACHE STRING 
    "Select the window system api to be compiled into the package." )

    set_property(CACHE MOTOR_TARGET_WINDOW_SYSTEM_API PROPERTY STRINGS "null" "win32" )

    if( MOTOR_TARGET_WINDOW_SYSTEM_API STREQUAL "null" )
        
        set( MOTOR_WINDOW_SYSTEM_NULL ON )
        set( MOTOR_WINDOW_SYSTEM_DEFINES -DMOTOR_WINDOW_SYSTEM_NULL )

    elseif( MOTOR_TARGET_WINDOW_SYSTEM_API STREQUAL "win32" )
    
        set( MOTOR_WINDOW_SYSTEM_NULL OFF )
        set( MOTOR_WINDOW_SYSTEM_WIN32 ON )
        set( MOTOR_WINDOW_SYSTEM_DEFINES -DMOTOR_WINDOW_SYSTEM_WIN32 )

    else()
        message( FATAL_ERROR "Invalid window system parameter" )
    endif()
    
elseif( MOTOR_TARGET_OS_LIN )
    
    find_package( X11 )
        
    if( X11_FOUND )
      
        set( MOTOR_TARGET_WINDOW_SYSTEM_API "x11" )
      
        set( MOTOR_WINDOW_SYSTEM_XLIB ON )
        set( MOTOR_WINDOW_SYSTEM_DEFINES -DMOTOR_WINDOW_SYSTEM_XLIB )
    
    elseif( MOTOR_TARGET_WINDOW_SYSTEM_API STREQUAL "wayland" )

        set( MOTOR_TARGET_WINDOW_SYSTEM_API "wayland" )
      
        set( MOTOR_WINDOW_SYSTEM_WAYLAND ON )
        set( MOTOR_WINDOW_SYSTEM_DEFINES -DMOTOR_WINDOW_SYSTEM_WAYLAND )
        message( "Wayland window system driver used" )
        message( FATAL_ERROR "Not supported at the moment" )

    else()
        message( FATAL_ERROR "Invalid window system parameter" )
    endif()

else()
    message( FATAL_ERROR "Window System Required" )
endif()


#
# Print Info
# 
if( MOTOR_WINDOW_SYSTEM_WIN32 )
    message( STATUS "[Window System] : Win32    " )
elseif( MOTOR_WINDOW_SYSTEM_XLIB )
    message( STATUS "[Window System] : XLib" )
elseif( MOTOR_WINDOW_SYSTEM_WAYLAND )
    message( STATUS "[Window System] : Wayland" )
else()
    message( STATUS "[Window System] : Unknown" )
endif()

set( MOTOR_WINDOW_SYSTEM_CONFIGURED TRUE )


