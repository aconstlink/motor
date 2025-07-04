cmake_minimum_required( VERSION 3.20 )

set( MOTOR_MAJOR_VERSION 0 )
set( MOTOR_MINOR_VERSION 0 )
set( MOTOR_PATCH_VERSION 2 )
set( MOTOR_VERSION ${MOTOR_MAJOR_VERSION}.${MOTOR_MINOR_VERSION}.${MOTOR_PATCH_VERSION} )

project( motor LANGUAGES C CXX VERSION ${MOTOR_VERSION})

if( PROJECT_SOURCE_DIR STREQUAL PROJECT_BINARY_DIR )
    message( FATAL_ERROR "In-source builds are not allowed." ) 
endif()
message( STATUS "Out-of-source build" ) 

if( CMAKE_CXX_STANDARD LESS 17 )
    message( FATAL_ERROR "C++17 compiler required.")
endif()

message( STATUS "C++ standard ok : " ${CMAKE_CXX_STANDARD} )

set( motor_root_dir ${CMAKE_CURRENT_LIST_DIR} )

file( REMOVE ${MOTOR_BINARY_DIR}/motor-targets.cmake  )
file( REMOVE OUTPUT ${MOTOR_BINARY_DIR}/motor-external-targets.cmake  )

################################################################
# SECTION: Configuring
################################################################

set( MOTOR_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR} )
set( MOTOR_BINARY_DIR ${PROJECT_BINARY_DIR} )
set_property( GLOBAL PROPERTY USE_FOLDERS ON )

set( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} 
  "${CMAKE_CURRENT_LIST_DIR}/cmake"
  "${CMAKE_CURRENT_LIST_DIR}/cmake/modules/"
  "${CMAKE_CURRENT_LIST_DIR}/cmake/configs"
  "${CMAKE_CURRENT_LIST_DIR}/cmake/macros/"
  "${CMAKE_CURRENT_LIST_DIR}/cmake/functions/"
  )

set( CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/install/motor-${MOTOR_VERSION}" CACHE 
    STRING "Install prefix" FORCE )

include( install_headers )
include( install_library )

include( config_export_target )
include( config_architecture )
include( config_operating_system )
include( config_compiler )
include( config_compiler_flags )

include( config_instruction_set )
include( config_graphics )

include( config_build_environment )
include( config_build_library_type )
include( config_window_system )
include( config_build_configuration )

include( config_rt_link ) 
include( config_output_directory )

include( visual_studio_src_dir )

#include( install_shaders )

include( emacs_default_directory )
motor_emacs_default_directory( ${motor_root_dir} )

#include( InstallRequiredSystemLibraries )

motor_export_clear()

################################################################
# SECTION : Build in and compilable externals
################################################################

add_subdirectory( "externals/kcat" )
add_subdirectory( "externals/ocornut" )
add_subdirectory( "externals/epezent" )
add_subdirectory( "externals/lua" )
add_subdirectory( "externals/nothings" )
add_subdirectory( "externals/aconstlink" )
add_subdirectory( "externals/nlohmann" )
add_subdirectory( "externals/nelarius" )
add_subdirectory( "externals/jkuhlmann" )

################################################################
# SECTION: find externals
################################################################

# optionals
#find_package( OpenCtm )
#find_package( Fmod ) 

if( MOTOR_TARGET_OS_WIN )
    find_package( XInput REQUIRED )
    find_package( XAudio2 REQUIRED )
 #   find_package( Wddk REQUIRED )
elseif( MOTOR_OS_LINUX )
endif()

if( MOTOR_WINDOW_SYSTEM_XLIB )
    #find_package( XLib )
endif()

################################################################
# SECTION: add sub directories
################################################################

set( subdirs_header_only 
    "base"
    "std"
    "math"
    
    
    #"collide"
    )

set( subdirs  )
set( opengl_subdir )
if( TARGET OpenGL::GL )
    list( APPEND opengl_subdir "ogl" )
endif()


list( APPEND subdirs
    "core"
    "log"
    "memory"
    "profiling"
    "property"
    "concurrent"
    "io"
    "controls"
    "msl"
    
    ${opengl_subdir}

    "network"
    "graphics"
    "audio"
    
    "platform"
    "application"    
    "tool"
    "font"
    "format"

    #"system"
    "gfx"
    "geometry"
    "noise"
    "social"
    "scene"
    "wire"
    "physics"
    #${subdirs_header_only}
    )



add_library( complete INTERFACE )
add_library( ${PROJECT_NAME}::complete ALIAS complete )

#
# header-only
#
foreach( _subdir ${subdirs_header_only} )
    
    set( cur_lib_name "${_subdir}" )
    set( cur_alias_name "${PROJECT_NAME}::${_subdir}" )
    set( cur_file_name "${PROJECT_NAME}_${_subdir}" )

    add_subdirectory( ${_subdir} )
    add_library( ${cur_alias_name} ALIAS ${cur_lib_name} )

    set_target_properties( ${cur_lib_name} PROPERTIES LINKER_LANGUAGE CXX )
    set_target_properties( ${cur_lib_name} PROPERTIES OUTPUT_NAME ${cur_file_name} )
    #set_target_properties( ${cur_lib_name} PROPERTIES CXX_STANDARD 17 ) #${MOTOR_CXX_STANDARD} )
    #set_property( TARGET ${cur_lib_name} PROPERTY CXX_STANDARD 17 )

    #target_compile_options( ${cur_lib_name} INTERFACE ${MOTOR_COMPILER_OPTIONS_PRIVATE})
    target_include_directories( ${cur_lib_name} INTERFACE 
        $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/..> 
        $<INSTALL_INTERFACE:include>  # <prefix>/include)
)
    
    if( MSVC_IDE )
        set_target_properties( ${cur_lib_name} PROPERTIES FOLDER "motor" )
    endif()

    target_link_libraries( complete INTERFACE ${cur_lib_name} )

    export( TARGETS ${cur_lib_name}
      NAMESPACE motor::
      APPEND FILE motor-targets.cmake
      )
endforeach()

#
# full libs
#
foreach( _subdir ${subdirs} )

    set( cur_lib_name_short "${_subdir}" )
    set( cur_lib_name "${_subdir}" )
    string( TOUPPER ${cur_lib_name} cur_lib_name_upper)
    set( cur_dir_name "${_subdir}" )
    set( cur_dir_name_raw "${_subdir}" )
    set( cur_alias_name "${PROJECT_NAME}::${_subdir}" )
    set( cur_file_name "${PROJECT_NAME}_${_subdir}" )

    set( LIB_NAME_UPPER ${cur_lib_name_upper} )
    configure_file( cmake/configure/api.in.h ${CMAKE_CURRENT_LIST_DIR}/${_subdir}/api.h @ONLY )
    unset( LIB_NAME_UPPER )

    add_subdirectory( ${_subdir} )

    set_target_properties( ${cur_lib_name} PROPERTIES LINKER_LANGUAGE CXX )
    set_target_properties( ${cur_lib_name} PROPERTIES OUTPUT_NAME ${cur_file_name} )
    #set_target_properties( ${cur_lib_name} PROPERTIES CXX_STANDARD 17 ) #${MOTOR_CXX_STANDARD} )
    #set_property( TARGET ${cur_lib_name} PROPERTY CXX_STANDARD 17 )

    if( MOTOR_LIBRARY_BUILD_SHARED )
      target_compile_options( ${cur_lib_name} 
        PRIVATE -DMOTOR_${cur_lib_name_upper}_EXPORT 
        INTERFACE -DMOTOR_${cur_lib_name_upper}_IMPORT )    
    endif()

    #target_compile_options( ${cur_lib_name} PRIVATE ${MOTOR_COMPILER_OPTIONS_PRIVATE})
    target_include_directories( ${cur_lib_name} 
      PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/..>
      PUBLIC $<INSTALL_INTERFACE:include>)
    
    set_target_properties( ${cur_lib_name} PROPERTIES FOLDER "motor" )
    
    target_link_libraries( complete INTERFACE ${cur_lib_name} )
    export( TARGETS ${cur_lib_name}
      NAMESPACE motor::
      APPEND FILE motor-targets.cmake
      )
endforeach()
#export( TARGETS complete NAMESPACE motor:: APPEND FILE ${PROJECT_NAME}-targets.cmake)


################################################################
# SECTION : Extensions
################################################################

################################################################
# SECTION : Export externals
################################################################

################################################################
# SECTION: in build tree fing_package generation
################################################################
configure_file( 
    "${MOTOR_SOURCE_DIR}/cmake/motor-config.build.in.cmake"
    "${MOTOR_BINARY_DIR}/motor-config.cmake"
    @ONLY )

#
#  Install tree 
#
#install( 
 #   FILES ${MOTOR_BINARY_DIR}/${PROJECT_NAME}-config.cmake 
  #  DESTINATION cmake )

#install( EXPORT ${PROJECT_NAME}-targets DESTINATION "cmake" )
