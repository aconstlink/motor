
#
# natus libraries
# Automatically generated via CMake's configure_file for the build tree.
#

get_filename_component( _dir "${CMAKE_CURRENT_LIST_FILE}" PATH )
get_filename_component( _prefix "${_dir}/.." ABSOLUTE )

####################################################################
# Import targets
####################################################################
include( "${_prefix}/cmake/natus-targets.cmake" )

####################################################################
# Directories
####################################################################

set( NATUS_CONFIGS_DIR 
    "${_prefix}/cmake/config" )

set( NATUS_MODULES_DIR 
    "${_prefix}/cmake/modules"
    "${_prefix}/cmake/macros"
    "${_prefix}/cmake/functions")
