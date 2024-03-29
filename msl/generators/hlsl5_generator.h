
#pragma once

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
        namespace hlsl
        {
            // the hlsl backend code generator
            class MOTOR_MSL_API hlsl5_generator
            {
                motor_this_typedefs( hlsl5_generator ) ;

            public:

                hlsl5_generator( void_t ) noexcept {}
                hlsl5_generator( motor::msl::generatable_rref_t ) noexcept {}
                hlsl5_generator( this_cref_t ) noexcept {}
                hlsl5_generator( this_rref_t ) noexcept {}
                ~hlsl5_generator( void_t ) {}

            public:
                
                static motor::string_t map_variable_type( motor::msl::type_cref_t type ) noexcept ;

                static motor::string_t replace_types( motor::string_t code ) noexcept ;

                static motor::string_t map_variable_binding( motor::msl::shader_type const st,
                    motor::msl::flow_qualifier const fq, motor::msl::binding binding ) noexcept ;

                motor::msl::generated_code_t::shaders_t generate( motor::msl::generatable_cref_t genable_, motor::msl::variable_mappings_cref_t var_map_ ) noexcept ;

                motor::msl::generated_code_t::code_t generate( motor::msl::generatable_cref_t genable, 
                    motor::msl::post_parse::config_t::shader_cref_t s, motor::msl::variable_mappings_cref_t var_mappings, motor::msl::api_type const type ) noexcept ;
            };
            motor_typedef( hlsl5_generator ) ;
        }
    }
}