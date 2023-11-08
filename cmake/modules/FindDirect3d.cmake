if( D3D_FOUND )
  return()
endif()

set( D3D_FOUND False )

message( STATUS ">>> FindDirect3d.cmake >>>" )

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

find_path( D3D_INCLUDE_DIRECTORY 
    NAMES "d3d10.h" "d3d10_1.h" "d3d11.h"
    HINTS ${SEARCH_D3D_INCLUDE_PATHS} 
    )

if( NOT D3D_INCLUDE_DIRECTORY )
    message( FATAL_ERROR "Direct3d include directory required. This may be due to the platform SDK path not found." ) 
endif()

set( D3D_INCLUDE_DIRS 
    ${D3D_INCLUDE_DIRECTORY}
)

#
# Search Direct3D 10
#
find_library( D3D10_LIBRARY_GENERAL 
    NAMES "d3d10" "d3d10_1" "d3dcompiler"
    HINTS ${D3D_LIBRARIES_SEARCH_PATHS} 
    )

find_library( D3D_COMPILER_GENERAL 
    NAMES "d3dcompiler"
    HINTS ${D3D_LIBRARIES_SEARCH_PATHS} 
    )

if( D3D10_LIBRARY_GENERAL )
    add_library( d3d10 INTERFACE )
    target_include_directories( d3d10 INTERFACE ${D3D_INCLUDE_DIRS} )
    target_link_libraries( d3d10 INTERFACE ${D3D10_LIBRARY_GENERAL} INTERFACE ${D3D_COMPILER_GENERAL} )
    message( STATUS "[Direct3d] : Target available : d3d10" )
endif()

#
# Search Direct3D 11
#
find_library( D3D11_LIBRARY_GENERAL 
    NAMES "d3d11"
    HINTS ${D3D_LIBRARIES_SEARCH_PATHS} 
    )

find_library( D3D_COMPILER_GENERAL 
    NAMES "d3dcompiler"
    HINTS ${D3D_LIBRARIES_SEARCH_PATHS} 
    )

# for reflection interface
find_library( D3D_DXGUID_GENERAL 
    NAMES "dxguid"
    HINTS ${D3D_LIBRARIES_SEARCH_PATHS} 
    )

if( D3D11_LIBRARY_GENERAL )
    add_library( d3d11 INTERFACE )
    target_include_directories( d3d11 INTERFACE ${D3D_INCLUDE_DIRS} )
    target_link_libraries( d3d11 
      INTERFACE ${D3D11_LIBRARY_GENERAL} 
      INTERFACE ${D3D_COMPILER_GENERAL} 
      INTERFACE ${D3D_DXGUID_GENERAL} )
    message( STATUS "[Direct3d] : Target available : d3d11" )
endif()

unset( D3D_INCLUDE_DIRECTORY CACHE )
unset( D3D10_LIBRARY_GENERAL CACHE )
unset( D3D11_LIBRARY_GENERAL CACHE )

set( D3D_FOUND True )

message( STATUS "<<< FindDirect3d.cmake <<<" )
