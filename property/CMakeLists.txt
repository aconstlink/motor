
set( sources 
    "typedefs.h"

    "property_sheet.hpp"
    "iproperty.hpp"
    "generic_property.hpp"
   
    )

motor_vs_src_dir( sources )

add_library( ${cur_lib_name} INTERFACE ${sources} )

target_link_libraries( ${cur_lib_name}
    INTERFACE motor::std
    INTERFACE motor::memory
    INTERFACE motor::log 
    INTERFACE motor::core
    )

###########################################################
# SECTION: Build Tree
###########################################################

##motor_export( ${cur_lib_name} )

###########################################################
# SECTION: Install
###########################################################

install_headers( "${sources}" "include/${PROJECT_NAME}/${cur_lib_name}" )
install_library( ${cur_lib_name} ${PROJECT_NAME}-targets )

