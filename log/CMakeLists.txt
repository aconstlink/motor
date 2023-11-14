
set( sources 
    "api.h"
    "result.h"
    "typedefs.h"
    "function.h"

    "global.h"
    "global.cpp"
    "log_level.h"

    "isystem.h"
    "system/system.h"
    "system/system.cpp"
    
    "ilogger.h"
    "logger/std_cerr_logger.h"
    "logger/std_cerr_logger.cpp"
    "logger/store_logger.h"
    "logger/store_logger.cpp"
    )

motor_vs_src_dir( sources )

add_library( ${cur_lib_name} ${MOTOR_LIBRARY_BUILD_TYPE} ${sources} )
add_library( ${cur_alias_name} ALIAS ${cur_lib_name} ) 

target_link_libraries( ${cur_lib_name} 
    INTERFACE motor::std
    PUBLIC motor::memory 
    )

###########################################################
# SECTION: Build Tree
###########################################################

#motor_export( ${cur_lib_name} )

###########################################################
# SECTION: Install
###########################################################

install_headers( "${sources}" "include/${PROJECT_NAME}/${cur_lib_name}" )
install_library( ${cur_lib_name} ${PROJECT_NAME}-targets )


