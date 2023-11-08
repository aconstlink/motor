
set( XINPUT_FOUND False )

message( STATUS ">>> FindXInput.cmake >>>" )

if( NATUS_TARGET_ARCH_BIT_64 )
  set( INT_ARCH "/x64" ) 
endif()

#see config_platform_sdk.cmake
set( SEARCH_XINPUT_INCLUDE_PATHS 
  ${NATUS_WINSDK_INCLUDE_PATHS}
  )

#see config_platform_sdk.cmake
set( XINPUT_LIBRARIES_SEARCH_PATHS 
  ${NATUS_WINSDK_LIBRARY_PATHS}
  )

message( STATUS "[XInput] : Looking in:" )
message( STATUS "[XInput] : ${NATUS_WINSDK_INCLUDE_PATHS}" )
message( STATUS "[XInput] : ${NATUS_WINSDK_LIBRARY_PATHS}" )

find_path( XINPUT_INCLUDE_DIRECTORY 
    NAMES "Xinput.h"
    HINTS ${SEARCH_XINPUT_INCLUDE_PATHS} 
    )

if( NOT XINPUT_INCLUDE_DIRECTORY )
    message( FATAL_ERROR "XInput include directory required. This may be due to the platform SDK path not found." ) 
endif()

find_library( XINPUT_LIBRARY_GENERAL 
    NAMES "xinput" 
    HINTS ${XINPUT_LIBRARIES_SEARCH_PATHS} 
    )

if( NOT XINPUT_LIBRARY_GENERAL )
    message( FATAL_ERROR "XInput Library required. This may be due to the platform SDK path not found." )
endif()

add_library( xinput INTERFACE )
target_include_directories( xinput INTERFACE ${XINPUT_INCLUDE_DIRECTORY} )
target_link_libraries( xinput INTERFACE ${XINPUT_LIBRARY_GENERAL} )
message( STATUS "[XInput] : Target available : xinput" )

unset( XINPUT_INCLUDE_DIRECTORY CACHE )
unset( XINPUT_LIBRARY_GENERAL CACHE )

set( XINPUT_FOUND True )

message( STATUS "<<< FindXInput.cmake <<<" )
