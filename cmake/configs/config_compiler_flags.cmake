

message( STATUS "[config] : compiler flags")

if( NOT MOTOR_COMPILER_CONFIGURED )
    message( FATAL_ERROR "Compiler not configured." )
endif()

# ct_ : means config target
set( THIS_TARGET ct_motor_compiler_options )
add_library( ${THIS_TARGET} INTERFACE )

if( MOTOR_COMPILER_MSC )
    
    # cherry-picking
    # C4005 : macro redefinition. Error within 3rd parties
    # C4100 : unreferenced local variable. Got error within 3rd parties
    # C4127 : conditional expression is constant. silence too inconvenient warnings.
    # C4251 : exporting stl container when building dlls
    # C4505 : unreferenced local function, e.g. unused function warning
    # C4702 : unreachable code hit in 3rd parties.
    # C4706 : assignment within conditional expression
    set( MOTOR_UNWANTED_COMPILER_OPTIONS 
	/wd4005 /wd4100 /wd4127 /wd4189 /wd4200 /wd4251 /wd4505 /wd4702 /wd4706 )

    # W4 : Warning level 4, e.g. nearly all warnings
    # WX : all compiler warnings as errors
    # EH : exception handling
    set( MOTOR_COMPILER_OPTIONS_PRIVATE /W4 /WX /EHsc ${MOTOR_UNWANTED_COMPILER_OPTIONS} )

    

    target_compile_options( ${THIS_TARGET} INTERFACE ${MOTOR_COMPILER_OPTIONS_PRIVATE} )

    unset( MOTOR_UNWANTED_COMPILER_OPTIONS CACHE ) 
    unset( MOTOR_COMPILER_OPTIONS_PRIVATE CACHE ) 

    #message( STATUS "Configured Microsoft Compiler Flags")

elseif( MOTOR_COMPILER_GNU )

    message( STATUS "[compiler] : GNU Compiler Flags") 

    set( MOTOR_UNWANTED_COMPILER_OPTIONS "-Wno-unused-function " )

    set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${MOTOR_UNWANTED_COMPILER_OPTIONS} -fcommon")
    set( CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS} ${MOTOR_UNWANTED_COMPILER_OPTIONS} -fcommon")
    set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${MOTOR_UNWANTED_COMPILER_OPTIONS} -Wno-pessimizing-move -Wno-redundant-move -fcommon")

    set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic")
    set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++11")

    set( CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} -O2 -g")
    set( CMAKE_CXX_FLAGS_RELEASE " ${CMAKE_CXX_FLAGS_RELEASE} -O2")


    set( CMAKE_CXX_FLAGS_DEBUG  "${CMAKE_CXX_FLAGS_DEBUG} -O0 -g")

    # todo
    set( MOTOR_COMPILER_OPTIONS_PRIVATE )
    target_compile_options( ${THIS_TARGET} INTERFACE "" )

    unset( MOTOR_UNWANTED_COMPILER_OPTIONS )

    #message( STATUS "GCC-C++ Flags: ${CMAKE_CXX_FLAGS}")

else()
    message( "No compiler flags configured due to unknown compiler.")
endif()

install( TARGETS ${THIS_TARGET} 
        EXPORT ${PROJECT_NAME}-targets )


