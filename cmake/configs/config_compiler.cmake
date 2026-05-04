

#
# determine the compiler
# for cross compilation, the target platform is required to be known
# 
# the values here can be initialized using a toolchain file

set( MOTOR_COMPILER_CONFIGURED FALSE )

set( THIS_TARGET ct_motor_compiler )
add_library( ${THIS_TARGET} INTERFACE )

# 
# 2026 motor is updated to c++ 20.
#
# it need to be used in the target property CXX_STANDARD
set( MOTOR_CXX_STANDARD 20 )
set( CMAKE_CXX_STANDARD_REQUIRED 20 )

#message( ${CMAKE_CXX_COMPILER_ID} )
 
# Microsoft compiler
set( MOTOR_COMPILER_MSC OFF )

# @TODO do we still need this?
set( MOTOR_COMPILER_MSC_14 OFF ) # vs 2015
set( MOTOR_COMPILER_MSC_15 OFF ) # vs 2017
set( MOTOR_COMPILER_MSC_16 OFF ) # vs 2019
set( MOTOR_COMPILER_MSC_17 OFF ) # vs 2022
set( MOTOR_COMPILER_MSC_20 OFF ) # cs 2026

# Gnu compiler (GCC, GCC-C++)
set( MOTOR_COMPILER_GNU OFF )
set( MOTOR_COMPILER_CLANG OFF )

if( CMAKE_CXX_COMPILER_ID STREQUAL "MSVC" )

    set( MOTOR_COMPILER_MSC ON )
    
    if(  MSVC_VERSION GREATER 1929 )
      message( STATUS "[motor compiler] : MSVC Version > 1929" )
      set( MOTOR_COMPILER_MSC_20 on )
    else()
      message( FATAL_ERROR "MSVC compiler not supported anymore. Need" 
        ${MOTOR_CXX_VERSION} "C++ Standard")
    endif()

    target_compile_definitions( ${THIS_TARGET} INTERFACE -DMOTOR_COMPILER_MSC )
    set_property(TARGET ${THIS_TARGET} PROPERTY CXX_STANDARD ${MOTOR_CXX_STANDARD} )

elseif( CMAKE_CXX_COMPILER_ID STREQUAL "GNU" )

    if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS 13)
        message(FATAL_ERROR "GCC version must be at least 13! Found ${CMAKE_CXX_COMPILER_VERSION}")
    endif()

    message( STATUS "[motor compiler] : GNU GCC Version OK" )

    set( MOTOR_COMPILER_GNU ON )
    target_compile_definitions( ${THIS_TARGET} INTERFACE -DMOTOR_COMPILER_GNU )
    set_property(TARGET ${THIS_TARGET} PROPERTY CXX_STANDARD ${MOTOR_CXX_STANDARD} )

elseif( CMAKE_CXX_COMPILER_ID STREQUAL "Clang" )

    set( MOTOR_COMPILER_CLANG ON )
    target_compile_definitions( ${THIS_TARGET} INTERFACE -DMOTOR_COMPILER_CLANG )
    set_property(TARGET ${THIS_TARGET} PROPERTY CXX_STANDARD ${MOTOR_CXX_STANDARD} )

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
set( CMAKE_CXX_EXTENSIONS OFF )

set( MOTOR_COMPILER_CONFIGURED TRUE )

export( TARGETS ${THIS_TARGET}
  NAMESPACE ${PROJECT_NAME}::
  APPEND FILE ${MOTOR_BINARY_DIR}/${PROJECT_NAME}-targets.cmake )

install( TARGETS ${THIS_TARGET} EXPORT ${PROJECT_NAME}-targets )
