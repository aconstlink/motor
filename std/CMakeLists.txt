
set( sources 

    "string"
    "vector"
    "map"
    "unordered"
    "filesystem"
    "stack"
    "sstream"
    "list"
    "insertion_sort.hpp"
    #"string/utf8.hpp"
    "string_split.hpp"
    )

motor_vs_src_dir( sources )

add_library( ${cur_lib_name} INTERFACE ${sources} )

target_link_libraries( ${cur_lib_name}
    INTERFACE motor::memory
    INTERFACE motor::core
    INTERFACE ghc_filesystem
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



