
#pragma once

#include "../api.h"
#include "../typedefs.h"

#include <motor/std/string>
#include <motor/math/vector/vector3.hpp>
#include <motor/core/document.hpp>

namespace motor
{
    namespace msl
    {
        class MOTOR_MSL_API forward_rendering_shader
        {
            motor_this_typedefs( forward_rendering_shader ) ;

        private:

            motor::core::document _doc ;

        public:

            struct generator_info
            {
                motor::string_t name ;

                byte_t normal_comps ;
                byte_t texcoord_comps ;


                // only 1 or 0 supported
                byte_t num_lights ;

                bool_t use_ambient ;
                motor::math::vec3f_t ambient ;

                bool_t use_diffuse ;
                motor::math::vec3f_t diffuse ;

                bool_t has_normals( void_t ) const noexcept
                {
                    return normal_comps > 0 ;
                }

                bool_t has_texcoords( void_t ) const noexcept
                {
                    return texcoord_comps > 0 ;
                }

                bool_t has_any_light( void_t ) const noexcept
                {
                    return num_lights > 0 ;
                }
            };
            motor_typedef( generator_info ) ;


            forward_rendering_shader( generator_info_rref_t ) noexcept ;
            forward_rendering_shader( this_rref_t ) noexcept ;
            ~forward_rendering_shader( void_t ) noexcept ;
            

            motor::string_t to_string( void_t ) const noexcept ;
            
        };
    }
}