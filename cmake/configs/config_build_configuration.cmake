
#
# Configures the build configuration
#
set( MOTOR_BUILD_CONFIG_CONFIGURED FALSE )

#
# Required other config modules
#
if( NOT MOTOR_COMPILER_CONFIGURED )
    message( FATAL_ERROR "Config the build configuration requires the compiler to be configured." )
elseif( NOT MOTOR_BUILD_ENV_CONFIGURED )
    message( FATAL_ERROR "Config the build configuration requires the build environment to be configured." )
endif()

message( STATUS "[config] : build configuration" )

# Do we have microsoft visual studio, the build config requires all flags
# to be set in the compiler option variable because visual studio allows 
# multiple build configurations.
if( MOTOR_COMPILER_MSC )

    #set( CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} /DMOTOR_DEBUG" )
    #set( CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /DMOTOR_DEBUG" )

    #set( CMAKE_C_FLAGS_MINSIZEREL "${CMAKE_C_FLAGS_MINSIZEREL} /DMOTOR_RELEASE")	
    #set( CMAKE_CXX_FLAGS_MINSIZEREL "${CMAKE_CXX_FLAGS_MINSIZEREL} /DMOTOR_RELEASE")	

    #set( CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /DMOTOR_RELEASE")	

    #set( CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /DMOTOR_RELEASE")	
    #set( CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /DMOTOR_RELEASE")	

    #set( CMAKE_C_FLAGS_RELWITHDEBINFO "${CMAKE_C_FLAGS_RELWITHDEBINFO} /DMOTOR_RELEASE")
    #set( CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} /DMOTOR_RELEASE")

# with a make system, we can only have one configuration at a time.
elseif( MOTOR_COMPILER_GNU )

    #
    #
    #
    set( MOTOR_BUILD_CONFIG_DEBUG OFF )
    set( MOTOR_BUILD_CONFIG_RELEASE OFF ) 

    #
    # Section: Preset based on CMake
    #
    if( CMAKE_BUILD_TYPE STREQUAL "Debug" )
        set( MOTOR_BUILD_CONFIG_NAME "Debug" )
    elseif( CMAKE_BUILD_TYPE STREQUAL "Release" )
        set( MOTOR_BUILD_CONFIG_NAME "Release" )
    else()
        set( MOTOR_BUILD_CONFIG_NAME "Debug" )
    endif()

    #
    # Section: User Option
    #
    set( MOTOR_BUILD_CONFIG_NAMES "Debug" "Release" )
    set( MOTOR_BUILD_CONFIG_NAME ${MOTOR_BUILD_CONFIG_NAME} CACHE STRING "Select your build configuration." )
    set_property( CACHE MOTOR_BUILD_CONFIG_NAME PROPERTY STRINGS ${MOTOR_BUILD_CONFIG_NAMES} )

    #
    # Reset CMake variable
    #
    set( CMAKE_BUILD_TYPE ${MOTOR_BUILD_CONFIG_NAME} CACHE STRING ${MOTOR_BUILD_CONFIG_NAME} FORCE )

    #
    # Section: Definitions
    #
    if( MOTOR_BUILD_CONFIG_NAME STREQUAL "Debug" )
        
        set( MOTOR_BUILD_CONFIG_DEBUG ON )
        #add_definitions( -DMOTOR_DEBUG )
        
    elseif( MOTOR_BUILD_CONFIG_NAME STREQUAL "Release" )

        set( MOTOR_BUILD_CONFIG_RELEASE ON )
        #add_definitions( -DMOTOR_RELEASE )
        
    endif()

endif()

message( STATUS "[done] : Build Configuration" )

set( MOTOR_BUILD_CONFIG_CONFIGURED TRUE )

