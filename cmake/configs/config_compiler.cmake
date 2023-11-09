

#
# determine the compiler
# for cross compilation, the target platform is required to be known
# 
# the values here can be initialized using a toolchain file

set( MOTOR_COMPILER_CONFIGURED FALSE )

# just set the cxx version to 11 for now
# it need to be used in the target property CXX_STANDARD
set( MOTOR_CXX_STANDARD 17 )

# Microsoft compiler
set( MOTOR_COMPILER_MSC OFF )
set( MOTOR_COMPILER_MSC_14 OFF ) # vs 2015
set( MOTOR_COMPILER_MSC_15 OFF ) # vs 2017
set( MOTOR_COMPILER_MSC_16 OFF ) # vs 2019
set( MOTOR_COMPILER_MSC_17 OFF ) # vs 2022

# Gnu compiler (GCC, GCC-C++)
set( MOTOR_COMPILER_GNU OFF )
set( MOTOR_COMPILER_CLANG OFF )

if( MSVC_IDE OR MSVC )

    set( MOTOR_COMPILER_MSC ON )
    
    if( MSVC_VERSION EQUAL 1900 )
      set( MOTOR_COMPILER_MSC_14 on )
      set( MOTOR_CXX_STANDARD 11 )
    elseif( MSVC_VERSION GREATER 1909 AND MSVC_VERSION LESS 1920 )
      set( MOTOR_COMPILER_MSC_15 on )
      set( MOTOR_CXX_STANDARD 14 )
    elseif( MSVC_VERSION GREATER 1919 AND MSVC_VERSION LESS 1930 )
      set( MOTOR_COMPILER_MSC_16 on )
      set( MOTOR_CXX_STANDARD 17 )
    elseif( MSVC_VERSION GREATER 1929 AND MSVC_VERSION LESS 1938 )
      set( MOTOR_COMPILER_MSC_17 on )
      set( MOTOR_CXX_STANDARD 17 )
    else()
      message( FATAL "MSVC Compiler not yet supported" )
    endif()

elseif( CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX )

    set( MOTOR_COMPILER_GNU ON )
    
else()

    message( FATAL_ERROR "Unsupported compiler")

endif()

#
# Print Info
#
if( MOTOR_COMPILER_MSC )

    message( STATUS "[compiler] : Microsoft Compiler Suite" )

elseif( MOTOR_COMPILER_GNU )

    message( STATUS "[compiler] : GNU Compiler Suite" )

endif()

set( CMAKE_CXX_STANDARD ${MOTOR_CXX_STANDARD} )
set( MOTOR_COMPILER_CONFIGURED TRUE )

