
if( XAUDIO2_FOUND )
    return()
endif()

set( XAUDIO2_FOUND FALSE )

if( NOT MOTOR_TARGET_OS_WIN )
  return()
endif()

# just set it to true for now
set( XAUDIO_ENABLE_HINT TRUE )
option( XAUDIO2_ENABLE "Enable DirectX XAudio2" ${XAUDIO_ENABLE_HINT} )

if( XAUDIO2_ENABLE )
    message( STATUS "[Enabled] : XAudio2" )
else()
    message( STATUS "[Disabled] : XAudio2" )
    return()
endif()

set( XAUDIO2_DEFINITIONS "-DXAUDIO2_ENABLE" )

# need to go through the directx sdk on windows 7
# no more windows 7 support
if( FALSE AND MOTOR_TARGET_OS_WIN7 )

  find_package( DirectXSdk )

  set( XAUDIO2_INCLUDEDIR ${DXSDK_INCLUDE_DIR} )
  set( XAUDIO2_LIBRARYDIR ${DXSDK_LIBRARY_DIR} )
  set( XAUDIO2_BINDIR ${DXSDK_BIN_DIR} )

  set( XAUDIO2_LIBRARIES winmm.lib ole32.lib )

  find_path( XAUDIO2_INCLUDE_DIRECTORY "xaudio2.h" HINTS ${XAUDIO2_INCLUDEDIR} )
  if( NOT XAUDIO2_INCLUDE_DIRECTORY )
    message( FATAL_ERROR "XAudio2.h not found in " ${XAUDIO2_INCLUDEDIR} ) 
  endif()

elseif( MOTOR_TARGET_OS_WIN8 )

elseif( MOTOR_TARGET_OS_WIN81 OR MOTOR_TARGET_OS_WIN10 )

  message( STATUS "[XAudio2] : Looking in:" )
  message( STATUS "[XAudio2] : ${MOTOR_WINSDK_INCLUDE_PATHS}" )
  message( STATUS "[XAudio2] : ${MOTOR_WINSDK_LIBRARY_PATHS}" )

  #see config_platform_sdk.cmake
  set( SEARCH_XAUDIO_INCLUDE_PATHS 
    ${MOTOR_WINSDK_INCLUDE_PATHS}
    )

  #see config_platform_sdk.cmake
  set( XAUDIO_LIBRARIES_SEARCH_PATHS 
    ${MOTOR_WINSDK_LIBRARY_PATHS}
    )

  find_path( XAUDIO2_INCLUDE_DIRECTORY "xaudio2.h" HINTS ${SEARCH_XAUDIO_INCLUDE_PATHS} )
  if( NOT XAUDIO2_INCLUDE_DIRECTORY )
    message( FATAL_ERROR "XAudio2.h not found in " ${XAUDIO2_INCLUDEDIR} ) 
  endif()

  find_library( XAUDIO2_LIB_GENERAL "xaudio2.lib" HINTS ${XAUDIO_LIBRARIES_SEARCH_PATHS} ) 
  if( NOT XAUDIO2_LIB_GENERAL )
    message( FATAL_ERROR "XAudio2.lib not found in " ${XAUDIO2_LIBRARYDIR} )
  endif()

  set( XAUDIO2_LIBRARIES winmm.lib ole32.lib ${XAUDIO2_LIB_GENERAL} )

  if( XAUDIO2_LIBRARY_GENERAL )
    add_library( xaudio2 INTERFACE )
    target_include_directories( xaudio2 INTERFACE ${XAUDIO2_INCLUDE_DIRECTORY} )
    target_link_libraries( xaudio2 INTERFACE ${XAUDIO2_LIBRARIES} )
    message( STATUS "[XAudio2] : Target available : XAudio2" )
  endif()

  unset( XAUDIO_LIBRARIES_SEARCH_PATHS CACHE )
  unset( SEARCH_XAUDIO_INCLUDE_PATHS CACHE )
  unset( XAUDIO2_INCLUDE_DIRECTORY CACHE )
  unset( XAUDIO2_LIB_GENERAL CACHE )
else()
  message( FATAL_ERROR "XAudio2 Path required" )
endif()

unset( XAUDIO2_DEFINITIONS CACHE )

set( XAUDIO2_FOUND TRUE )
