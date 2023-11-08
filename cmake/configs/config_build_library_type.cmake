

if( NOT NATUS_OS_CONFIGURED )
    message( FATAL_ERROR "OS must be configured" )
endif()

option( NATUS_LIBRARY_BUILD_SHARED "Build shared libraries?" ON )

if( NATUS_LIBRARY_BUILD_SHARED ) 
    message( STATUS "snakeoil library build type: shared" )
    set( NATUS_LIBRARY_BUILD_TYPE SHARED )
    # if we build a shared library or an application,
    # all external libs need to be linked.
    set( NATUS_EXTERNAL_LINKAGE_REQUIRED 1 )

    if( NATUS_TARGET_OS_WIN )
        add_definitions( -DNATUS_BUILD_DLL )
    endif()
else()
    message( STATUS "snakeoil library build type: static" )
    set( NATUS_LIBRARY_BUILD_TYPE STATIC )
    set( NATUS_EXTERNAL_LINKAGE_REQUIRED 0 )
endif()


