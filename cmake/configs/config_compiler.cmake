

#
# determine the compiler
# for cross compilation, the target platform is required to be known
# 
# the values here can be initialized using a toolchain file

set( MOTOR_COMPILER_CONFIGURED FALSE )

set( THIS_TARGET ct_motor_compiler )
add_library( ${THIS_TARGET} INTERFACE )

# just set the cxx version to 11 for now
# it need to be used in the target property CXX_STANDARD
set( MOTOR_CXX_STANDARD 17 )

message( ${CMAKE_CXX_COMPILER_ID} )
 
# Microsoft compiler
set( MOTOR_COMPILER_MSC OFF )
set( MOTOR_COMPILER_MSC_14 OFF ) # vs 2015
set( MOTOR_COMPILER_MSC_15 OFF ) # vs 2017
set( MOTOR_COMPILER_MSC_16 OFF ) # vs 2019
set( MOTOR_COMPILER_MSC_17 OFF ) # vs 2022

# Gnu compiler (GCC, GCC-C++)
set( MOTOR_COMPILER_GNU OFF )
set( MOTOR_COMPILER_CLANG OFF )

if( CMAKE_CXX_COMPILER_ID STREQUAL "MSVC" )

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
      set( MOTOR_COMPILER_MSC_17 on )
      set( MOTOR_CXX_STANDARD 17 )
    endif()

    target_compile_definitions( ${THIS_TARGET} INTERFACE -DMOTOR_COMPILER_MSC )

elseif( CMAKE_CXX_COMPILER_ID STREQUAL "GNU" )

    set( MOTOR_COMPILER_GNU ON )
    target_compile_definitions( ${THIS_TARGET} INTERFACE -DMOTOR_COMPILER_GNU )

elseif( CMAKE_CXX_COMPILER_ID STREQUAL "Clang" )

    set( MOTOR_COMPILER_CLANG ON )
    target_compile_definitions( ${THIS_TARGET} INTERFACE -DMOTOR_COMPILER_CLANG )

else()

    message( FATAL_ERROR "Unsupported compiler")
    target_compile_definitions( ${THIS_TARGET} INTERFACE -DMOTOR_COMPILER_UNKNOWN )

endif()

#
# Print Info
#
if( MOTOR_COMPILER_MSC )

    message( STATUS "[compiler] : Microsoft Compiler Suite" )

elseif( MOTOR_COMPILER_GNU )

    message( STATUS "[compiler] : GNU Compiler Suite" )

elseif( MOTOR_COMPILER_CLANG )

    message( STATUS "[compiler] : LLVM CLang" )

endif()

set( CMAKE_CXX_STANDARD ${MOTOR_CXX_STANDARD} )
set( MOTOR_COMPILER_CONFIGURED TRUE )

install( TARGETS ${THIS_TARGET} EXPORT ${PROJECT_NAME}-targets )
