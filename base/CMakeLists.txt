

set( sources 
    "types.hpp"
    "mtr_ownership.hpp"
    "macros/common.h"
    "macros/typedef.h"
    "macros/function.h"
    "macros/move.h"   
    )

motor_vs_src_dir( sources )

add_library( ${cur_lib_name} INTERFACE ${sources} )


target_compile_definitions( ${cur_lib_name} INTERFACE -DMOTOR_LITTLE_ENDIAN=0 )
target_compile_definitions( ${cur_lib_name} INTERFACE -DMOTOR_BIG_ENDIAN=1 )
target_compile_definitions( ${cur_lib_name} INTERFACE -DMOTOR_BYTE_ORDER=${MOTOR_TARGET_ARCH_BIG_ENDIAN} )

target_link_libraries( ${cur_lib_name} 
    INTERFACE ct_motor_operating_system 
    INTERFACE ct_motor_build_library_type
    INTERFACE ct_motor_compiler_options
    INTERFACE ct_motor_compiler )

###########################################################
# SECTION: Build Tree
###########################################################

#motor_export( ${CUR_LIB_NAME} )

###########################################################
# SECTION: Install
###########################################################

install_headers( "${sources}" "include/${PROJECT_NAME}/${cur_lib_name}" )
install_library( ${cur_lib_name} ${PROJECT_NAME}-targets )
