

set( sources 
    "api.h"
    "types.hpp"

    "std/string"
    "std/vector"
    "std/map"
    "std/unordered"

    "macros/common.h"
    "macros/typedef.h"
    "macros/function.h"
    "macros/move.h"

    "log/result.h"
    "log/typedefs.h"
    "log/function.h"

    "log/global.h"
    "log/global.cpp"
    "log/log_level.h"

    "log/isystem.h"
    "log/system/system.h"
    "log/system/system.cpp"
    
    "log/ilogger.h"
    "log/logger/std_cerr_logger.h"
    "log/logger/std_cerr_logger.cpp"
    "log/logger/store_logger.h"
    "log/logger/store_logger.cpp"

    "memory/typedefs.h"
    
    "memory/allocator.hpp"
    "memory/global.h"
    "memory/global.cpp"

    "memory/imanager.h"
    "memory/manager/manager.h"
    "memory/manager/manager.cpp"
    
    "memory/malloc_guard.hpp"
    
    "memory/arena.hpp"
    )

motor_vs_src_dir( sources )


add_library( ${cur_lib_name} ${MOTOR_LIBRARY_BUILD_TYPE} ${sources} )
add_library( ${cur_alias_name} ALIAS ${cur_lib_name} )

target_compile_definitions( ${cur_lib_name} PUBLIC -DMOTOR_LITTLE_ENDIAN=0 )
target_compile_definitions( ${cur_lib_name} PUBLIC -DMOTOR_BIG_ENDIAN=1 )
target_compile_definitions( ${cur_lib_name} PUBLIC -DMOTOR_BYTE_ORDER=${MOTOR_TARGET_ARCH_BIG_ENDIAN} )

###########################################################
# SECTION: Build Tree
###########################################################

#motor_export( ${CUR_LIB_NAME} )

###########################################################
# SECTION: Install
###########################################################

install_headers( "${sources}" "include/${PROJECT_NAME}/${cur_lib_name}" )
install_library( ${cur_lib_name} ${PROJECT_NAME}-targets )
