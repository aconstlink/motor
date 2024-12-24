

set( sources 
    "double_dispatch.hpp"
    "document.hpp"
    "dummy.cpp"
    )

motor_vs_src_dir( sources )

add_library( ${cur_lib_name} ${MOTOR_LIBRARY_BUILD_TYPE} ${sources} )
add_library( ${cur_alias_name} ALIAS ${cur_lib_name} ) 

target_link_libraries( ${cur_lib_name} 
    INTERFACE motor::base
    PUBLIC motor::memory
    PUBLIC motor::log
    PUBLIC motor::concurrent
    PUBLIC motor::std )

###########################################################
# SECTION: Build Tree
###########################################################

#motor_export( ${CUR_LIB_NAME} )

###########################################################
# SECTION: Install
###########################################################

install_headers( "${sources}" "include/${PROJECT_NAME}/${cur_lib_name}" )
install_library( ${cur_lib_name} ${PROJECT_NAME}-targets )
