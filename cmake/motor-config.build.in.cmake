
#
# motor libraries
# Automatically generated via CMake's configure_file for the build tree.
#

####################################################################
# Import targets
####################################################################
include( "${CMAKE_CURRENT_LIST_DIR}/motor-external-targets.cmake" )
include( "${CMAKE_CURRENT_LIST_DIR}/motor-targets.cmake" )

####################################################################
# Directories
####################################################################

get_filename_component( _dir "@CMAKE_CURRENT_LIST_FILE@" PATH )
get_filename_component( _prefix "${_dir}" ABSOLUTE )

set( MOTOR_CONFIGS_DIR 
    "${_prefix}/cmake/config" )

set( MOTOR_MODULES_DIR 
    "${_prefix}/cmake/modules"
    "${_prefix}/cmake/macros"
    "${_prefix}/cmake/functions")

set( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} 
  ${MOTOR_CONFIGS_DIR}
  ${MOTOR_MODULES_DIR}
  )

set( motor_root_dir ${_prefix} )
set( motor_bindir_debug @CMAKE_BINARY_DIR@/build/Debug )
set( motor_bindir_release @CMAKE_BINARY_DIR@/build/Release  )

include( visual_studio_src_dir )
include( visual_studio_generate_bat_dll )
