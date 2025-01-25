
#pragma once

#include "../api.h"
#include "../typedefs.h"

#include <motor/graphics/object/msl_object.h>
#include <motor/graphics/object/geometry_object.h>
#include <motor/graphics/object/framebuffer_object.h>

namespace motor
{
    namespace gfx
    {
        namespace postprocess
        {
            using namespace motor::core::types ;

            class MOTOR_GFX_API downsample
            {
                motor_this_typedefs( downsample ) ;

            private:

                motor::string_t _name ;
                motor::graphics::msl_object_t _msl ;
                motor::graphics::geometry_object_t _geo ; // post quad
                motor::graphics::framebuffer_object_t _fb[3] ; // 3x downsample

            public:

                downsample( motor::string_in_t name ) noexcept ;
                downsample( this_cref_t ) = delete ;
                downsample( this_rref_t ) noexcept ;
                ~downsample( void_t ) noexcept ;

            public:

                void_t init( motor::math::vec2ui_in_t target_dims ) noexcept ;
                void_t release( void_t ) noexcept ;
                
                // This call will execute the downsample shaders
                void_t execute( void_t ) noexcept ;
            };
        }
    }
}