

if( NOT MOTOR_OS_CONFIGURED )
    message( FATAL_ERROR "OS must be configured" )
endif()

option( MOTOR_LIBRARY_BUILD_SHARED "Build shared libraries?" ON )

# ct_ : means config target
set( THIS_TARGET ct_motor_build_library_type )
add_library( ${THIS_TARGET} INTERFACE )

if( MOTOR_LIBRARY_BUILD_SHARED ) 
    message( STATUS "motor library build type: shared" )
    set( MOTOR_LIBRARY_BUILD_TYPE SHARED )
    # if we build a shared library or an application,
    # all external libs need to be linked.
    set( MOTOR_EXTERNAL_LINKAGE_REQUIRED 1 )

    if( MOTOR_TARGET_OS_WIN )
        target_compile_definitions( ${THIS_TARGET} INTERFACE 
        -DMOTOR_BUILD_DLL )
    endif()
else()
    message( STATUS "motor library build type: static" )
    set( MOTOR_LIBRARY_BUILD_TYPE STATIC )
    set( MOTOR_EXTERNAL_LINKAGE_REQUIRED 0 )
endif()

install( TARGETS ${THIS_TARGET} 
        EXPORT ${PROJECT_NAME}-targets )
