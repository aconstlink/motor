
set( sources 
    "typedefs.h"

    "string"
    "vector"
    "map"
    "unordered"

    #"vector.hpp"
    #"list.hpp"
    #"map.hpp"
    #"stack.hpp"

    #"filesystem.hpp"
    #"string.hpp"

    #"insertion_sort.hpp"

    #"string/utf8.hpp"
    #"string/split.hpp"

    "header_only.cpp"
    )

motor_vs_src_dir( sources )

add_library( ${cur_lib_name} ${MOTOR_LIBRARY_BUILD_TYPE} ${sources} )
add_library( ${cur_alias_name} ALIAS ${cur_lib_name} )

target_link_libraries( ${cur_lib_name}
        PUBLIC motor::memory
        PUBLIC motor::core
        PUBLIC ghc_filesystem
)

###########################################################
# SECTION: Build Tree
###########################################################

#motor_export( ${to_be_included} )

###########################################################
# SECTION: Install
###########################################################

install_headers( "${sources}" "include/${PROJECT_NAME}/${cur_lib_name}" )
install_library( ${cur_lib_name} ${PROJECT_NAME}-targets )



