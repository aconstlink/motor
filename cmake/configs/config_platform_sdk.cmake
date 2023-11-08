if( WINSDK_FOUND )
  return()
endif()

message( STATUS ">>> config_platform_sdk.cmake >>>" )

set( WINSDK_FOUND False )

if( MOTOR_TARGET_OS_WIN7 )

    message( STATUS "[WindowsSDK] : Win 7 SDK" )
    message( "[WindowsSDK] : Win 7 is outdated." )

    if( MOTOR_TARGET_ARCH_BIT_64 )
        set( INT_ARCH "/x64" ) 
    endif()

    set( SEARCH_WINSDK_INCLUDE_PATHS 
        "C:/Program Files (x86)/Microsoft SDKs/Windows/v7.1A/Include"
        "C:/Program Files (x86)/Microsoft SDKs/Windows/v7.0A/Include"
        $ENV{WINSDK_INCLUDEDIR_UM}
    )

    set( WINSDK_LIBRARIES_SEARCH_PATHS 
        "C:/Program Files (x86)/Microsoft SDKs/Windows/v7.1A/Lib${INT_ARCH}"
        "C:/Program Files (x86)/Microsoft SDKs/Windows/v7.0A/Lib${INT_ARCH}"
        $ENV{WINSDK_LIBRARYDIR_UM}
    )

elseif( MOTOR_TARGET_OS_WIN81 )

    message( STATUS "[WindowsSDK] : Win 8.1 SDK" )
    message( "[WindowsSDK] : Win 8 is outdated." )

    if( MOTOR_TARGET_ARCH_BIT_64 )
        set( INT_ARCH "/x64" ) 
    endif()

    set( SEARCH_WINSDK_INCLUDE_PATHS 
        "C:/Program Files (x86)/Windows Kits/8.1/Include/um"
        $ENV{WINSDK_INCLUDEDIR_UM}
    )

    set( WINSDK_LIBRARIES_SEARCH_PATHS 
        "C:/Program Files (x86)/Windows Kits/8.1/Lib/winv6.3/um${INT_ARCH}"
        $ENV{WINSDK_LIBRARYDIR_UM}
    )

elseif( MOTOR_TARGET_OS_WIN10 )

    message( STATUS "[WindowsSDK] : Win 10 SDK" )

    if( MOTOR_TARGET_ARCH_BIT_64 )
        set( INT_ARCH "/x64" )
    endif()

    #todo somehow allow user to add more paths
    set( sdk_bases "C:/Program Files (x86)/Windows Kits" )
    foreach( sdk_base ${sdk_bases} )

      # 1: check if the bases exist
      set( sdk_inc "${sdk_base}/10/Include" )
      set( sdk_lib "${sdk_base}/10/Lib" )
      if( NOT EXISTS ${sdk_inc} OR NOT EXISTS ${sdk_lib} )
        continue()
      endif()

      # 2: check if the subdirectories exist and select the right one
      set( sdk_version ${CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION} )
      #message( "${sdk_inc}/${sdk_version}" )
      if( NOT EXISTS "${sdk_inc}/${sdk_version}" )
        set( sdk_version "" )
        set( directories "" )
        subdirectories( directories ${sdk_inc} )
        list( FILTER directories INCLUDE REGEX "^10(\.[0-9])*" )
        list( REVERSE directories )
        foreach( subdir ${directories} )
          if( EXISTS ${sdk_inc}/${subdir} )
            set( sdk_version ${subdir} )
            break()
          endif()
        endforeach()

        unset( directories )
      endif()

      # 3: lets just assume, the default SDK install directory is given
      set( sdk_inc "${sdk_inc}/${sdk_version}" )
      set( sdk_lib "${sdk_lib}/${sdk_version}" )
      message( STATUS "[WindowsSDK] : ${sdk_inc}" )
      break()
    endforeach()
    unset( sdk_version )


    set( MOTOR_WINSDK_INCLUDE_PATHS 
        "${sdk_inc}/um"
        $ENV{WINSDK_INCLUDEDIR_UM}
    )

    set( MOTOR_WINSDK_LIBRARY_PATHS 
        "${sdk_lib}/um${INT_ARCH}"
        $ENV{WINSDK_LIBRARYDIR_UM}
    )

endif()

message( STATUS "<<< config_platform_sdk.cmake <<<" )

set( WINSDK_FOUND True )
