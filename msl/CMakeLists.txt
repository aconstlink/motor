

set( sources 
    "api.h"
    "typedefs.h"
    "enums.hpp"

    "database.hpp"
    "parser.h"
    "parser.cpp"
    "typedefs.h"
    "symbol_table.hpp"
    "symbol.hpp"
    "dependency_resolver.hpp"
    "function_declaration_analyser.hpp"
    "function_signature_analyser.hpp"
    "nodes.h"
    "nodes.cpp"

    "generators/generator.h"
    "generators/generator.cpp"
    "generators/generator_structs.hpp"
    "generators/essl3_generator.h"
    "generators/essl3_generator.cpp"
    "generators/glsl4_generator.h"
    "generators/glsl4_generator.cpp"
    "generators/hlsl5_generator.h"
    "generators/hlsl5_generator.cpp"

    "parser_structs.hpp"
    )

motor_vs_src_dir( sources )

add_library( ${cur_lib_name} ${MOTOR_LIBRARY_BUILD_TYPE} ${sources} )
add_library( ${cur_alias_name} ALIAS ${cur_lib_name} )


target_link_libraries( ${cur_lib_name}
        PUBLIC motor::concurrent
        PUBLIC motor::log
        PUBLIC motor::memory
        PUBLIC motor::std
        PUBLIC motor::core
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
