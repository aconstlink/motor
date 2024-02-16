#pragma once

#include "../api.h"
#include "../typedefs.h"

#include "line_render_3d.h"
#include "tri_render_3d.h"

namespace motor
{
    namespace gfx
    {
        class MOTOR_GFX_API primitive_render_3d
        {
            motor_this_typedefs( primitive_render_3d ) ;

        private: // other render

            motor::string_t _name ;
            motor::graphics::async_views_t _asyncs ;

            motor::gfx::line_render_3d_res_t _lr ;
            motor::gfx::tri_render_3d_res_t _tr ;

        public:

            primitive_render_3d( void_t ) ;
            primitive_render_3d( this_cref_t ) = delete ;
            primitive_render_3d( this_rref_t ) ;
            ~primitive_render_3d( void_t ) ;

            void_t init( motor::string_cref_t, motor::graphics::async_views_t ) noexcept ;
            void_t release( void_t ) noexcept ;

        public:

            void_t draw_line( motor::math::vec3f_cref_t p0, motor::math::vec3f_cref_t p1, motor::math::vec4f_cref_t color ) noexcept ;

            void_t draw_tri( motor::math::vec3f_cref_t p0, motor::math::vec3f_cref_t p1, motor::math::vec3f_cref_t p2, motor::math::vec4f_cref_t color ) noexcept ;

            #if 0
            void_t draw_rect( motor::math::vec3f_cref_t p0, motor::math::vec3f_cref_t p1, motor::math::vec3f_cref_t p2, motor::math::vec3f_cref_t p3, motor::math::vec4f_cref_t color, 
                motor::math::vec4f_cref_t border_color ) noexcept ;
            #endif
            void_t draw_circle( motor::math::mat3f_cref_t, motor::math::vec3f_cref_t p0, float_t const r, motor::math::vec4f_cref_t color, 
                motor::math::vec4f_cref_t border_color, size_t const ) noexcept ;

            void_t set_view_proj( motor::math::mat4f_cref_t view, motor::math::mat4f_cref_t proj ) noexcept ;

        public:

            // copy all data to the gpu buffer and transmit the data
            void_t prepare_for_rendering( void_t ) noexcept ;
            void_t render( void_t ) noexcept ;
        };
        motor_res_typedef( primitive_render_3d ) ;
    }
}