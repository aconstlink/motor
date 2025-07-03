
#
# motor libraries
# Automatically generated via CMake's configure_file for the build tree.
#

get_filename_component( _dir "@CMAKE_CURRENT_LIST_FILE@" PATH )
get_filename_component( _prefix "${_dir}/.." ABSOLUTE )

####################################################################
# Import targets
####################################################################
include( "${CMAKE_CURRENT_LIST_DIR}/motor-targets.cmake" )

####################################################################
# Directories
####################################################################

set( MOTOR_CONFIGS_DIR 
    "${_prefix}/cmake/config" )

set( MOTOR_MODULES_DIR 
    "${_prefix}/cmake/modules"
    "${_prefix}/cmake/macros"
    "${_prefix}/cmake/functions")
