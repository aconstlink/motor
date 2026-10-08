if( D3D_FOUND )
  return()
endif()

set( D3D_FOUND False )

message( STATUS ">>> FindDirect3d.cmake >>>" )

# Preserve existing SDK selections when migrating to MOTOR_* cache names.
foreach( old_name IN ITEMS D3D_INCLUDE_DIRECTORY D3D10_LIBRARY_GENERAL
    D3D11_LIBRARY_GENERAL D3D_COMPILER_GENERAL D3D_DXGUID_GENERAL )
  if( DEFINED CACHE{${old_name}} )
    if( NOT DEFINED CACHE{MOTOR_${old_name}} )
      get_property( old_value CACHE "${old_name}" PROPERTY VALUE )
      get_property( old_type CACHE "${old_name}" PROPERTY TYPE )
      get_property( old_help CACHE "${old_name}" PROPERTY HELPSTRING )
      set( MOTOR_${old_name} "${old_value}" CACHE ${old_type} "${old_help}" )
    endif()
    unset( ${old_name} CACHE )
  endif()
endforeach()
unset( old_name )
unset( old_value )
unset( old_type )
unset( old_help )

if( NOT MOTOR_WINSDK_INCLUDE_PATHS )
  message( "FildDirect3D Module requires the windows SDK" )
  message( "Please use  the Windows SDK module first" )
endif()

message( STATUS "Windows SDK provided. Can search for Direct3D" )

#see config_platform_sdk.cmake
set( SEARCH_D3D_INCLUDE_PATHS 
  ${MOTOR_WINSDK_INCLUDE_PATHS}
  )

#see config_platform_sdk.cmake
set( D3D_LIBRARIES_SEARCH_PATHS 
  ${MOTOR_WINSDK_LIBRARY_PATHS}
  )

message( STATUS "[Direct3d] : Looking in:" )
message( STATUS "[Direct3d] : ${MOTOR_WINSDK_INCLUDE_PATHS}" )
message( STATUS "[Direct3d] : ${MOTOR_WINSDK_LIBRARY_PATHS}" )

if( MOTOR_TARGET_ARCH_BIT_64 )
  set( INT_ARCH "/x64" ) 
endif()

find_path( MOTOR_D3D_INCLUDE_DIRECTORY
    NAMES "d3d10.h" "d3d10_1.h" "d3d11.h"
    HINTS ${SEARCH_D3D_INCLUDE_PATHS} 
    )

if( NOT MOTOR_D3D_INCLUDE_DIRECTORY )
    message( FATAL_ERROR "Direct3d include directory required. This may be due to the platform SDK path not found." ) 
endif()

set( D3D_INCLUDE_DIRS 
    ${MOTOR_D3D_INCLUDE_DIRECTORY}
)

#
# Search Direct3D 10
#
find_library( MOTOR_D3D10_LIBRARY_GENERAL
    NAMES "d3d10" "d3d10_1" "d3dcompiler"
    HINTS ${D3D_LIBRARIES_SEARCH_PATHS} 
    )

find_library( MOTOR_D3D_COMPILER_GENERAL
    NAMES "d3dcompiler"
    HINTS ${D3D_LIBRARIES_SEARCH_PATHS} 
    )

if( MOTOR_D3D10_LIBRARY_GENERAL )
    add_library( d3d10 INTERFACE )
    target_include_directories( d3d10 INTERFACE ${D3D_INCLUDE_DIRS} )
    target_link_libraries( d3d10 INTERFACE ${MOTOR_D3D10_LIBRARY_GENERAL} INTERFACE ${MOTOR_D3D_COMPILER_GENERAL} )
    message( STATUS "[Direct3d] : Target available : d3d10" )
endif()

#
# Search Direct3D 11
#
find_library( MOTOR_D3D11_LIBRARY_GENERAL
    NAMES "d3d11"
    HINTS ${D3D_LIBRARIES_SEARCH_PATHS} 
    )

find_library( MOTOR_D3D_COMPILER_GENERAL
    NAMES "d3dcompiler"
    HINTS ${D3D_LIBRARIES_SEARCH_PATHS} 
    )

# for reflection interface
find_library( MOTOR_D3D_DXGUID_GENERAL
    NAMES "dxguid"
    HINTS ${D3D_LIBRARIES_SEARCH_PATHS} 
    )

if( MOTOR_D3D11_LIBRARY_GENERAL )
    add_library( d3d11 INTERFACE )
    target_include_directories( d3d11 INTERFACE ${D3D_INCLUDE_DIRS} )
    target_link_libraries( d3d11 
      INTERFACE ${MOTOR_D3D11_LIBRARY_GENERAL}
      INTERFACE ${MOTOR_D3D_COMPILER_GENERAL}
      INTERFACE ${MOTOR_D3D_DXGUID_GENERAL} )
    message( STATUS "[Direct3d] : Target available : d3d11" )
endif()

unset( MOTOR_D3D_INCLUDE_DIRECTORY CACHE )
unset( MOTOR_D3D10_LIBRARY_GENERAL CACHE )
unset( MOTOR_D3D11_LIBRARY_GENERAL CACHE )

set( D3D_FOUND True )

message( STATUS "<<< FindDirect3d.cmake <<<" )
