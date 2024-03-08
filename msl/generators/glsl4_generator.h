#pragma once

#include "../typedefs.h"
#include "../parser_structs.hpp"

#include "generator_structs.hpp"

#include <motor/std/vector>
#include <motor/std/map>

namespace motor
{
    namespace msl
    {
        namespace glsl
        {
            // the glsl backend code generator
            class MOTOR_MSL_API glsl4_generator
            {
                motor_this_typedefs( glsl4_generator ) ;

            private:

                static motor::string_t replace_buildin_symbols( motor::string_rref_t code ) noexcept ;                
                static motor::string_t to_texture_type( motor::msl::type_cref_t t ) noexcept ;
                static motor::string_t replace_types( motor::msl::api_type const apit, motor::string_t code ) noexcept ;
                static motor::string_t determine_input_interface_block_name( motor::msl::shader_type const cur, motor::msl::shader_type const before ) noexcept ;
                static motor::string_t determine_output_interface_block_name( motor::msl::shader_type const cur, motor::msl::shader_type const after ) noexcept ;

            public:

                glsl4_generator( void_t ) noexcept {}
                glsl4_generator( this_cref_t ) noexcept {}
                glsl4_generator( this_rref_t ) noexcept {}
                ~glsl4_generator( void_t ) {}

            public:

                // 1. filter
                // 2. make variable names
                // 3. generate code
                motor::msl::generated_code_t::shaders_t generate( motor::msl::generatable_cref_t genable_, motor::msl::variable_mappings_cref_t var_map_ ) noexcept ;

                motor::msl::generated_code_t::code_t generate( motor::msl::generatable_cref_t genable, motor::msl::post_parse::config_t::shader_cref_t s, motor::msl::variable_mappings_cref_t var_mappings, motor::msl::api_type const type ) noexcept ;
            };
            motor_typedef( glsl4_generator ) ;
        }
    }
}