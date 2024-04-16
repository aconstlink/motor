#pragma once

#include "../api.h"
#include "../typedefs.h"

#include "line_render_2d.h"
#include "tri_render_2d.h"

namespace motor
{
    namespace gfx
    {
        class MOTOR_GFX_API primitive_render_2d
        {
            motor_this_typedefs( primitive_render_2d ) ;

        private: // other render

            motor::string_t _name ;

            motor::gfx::line_render_2d_t _lr ;
            motor::gfx::tri_render_2d_t _tr ;

        public:

            primitive_render_2d( void_t ) noexcept ;
            primitive_render_2d( this_cref_t ) = delete ;
            primitive_render_2d( this_rref_t ) noexcept;
            ~primitive_render_2d( void_t ) noexcept ;

            void_t init( motor::string_cref_t ) noexcept ;
            void_t release( void_t ) noexcept ;

        public:

            void_t draw_line( size_t const layer, motor::math::vec2f_cref_t p0, motor::math::vec2f_cref_t p1, motor::math::vec4f_cref_t color ) noexcept ;
            
            
            
            void_t draw_lines( size_t const layer, size_t const num_lines, motor::gfx::line_render_2d::draw_lines_funk_t ) noexcept ;

            void_t draw_tri( size_t const, motor::math::vec2f_cref_t p0, motor::math::vec2f_cref_t p1, 
                motor::math::vec2f_cref_t p2, motor::math::vec4f_cref_t color ) noexcept ;

            void_t draw_tris( size_t const layer, size_t const num_tris, motor::gfx::tri_render_2d::draw_tris_funk_t ) noexcept ;
            
            struct circle
            {
                motor::math::vec2f_t pos ;
                float_t radius ;
                motor::math::vec4f_t color ;
                motor::math::vec4f_t border ;
            };
            using circle_t = circle ;
            using draw_circles_funk_t = std::function< circle_t ( size_t const i ) > ;

            void_t draw_circles( size_t const layer, size_t const segs, 
                size_t const num_circles, this_t::draw_circles_funk_t ) noexcept ;
            void_t draw_circles_border( size_t const layer, size_t const segs,
                size_t const num_circles, this_t::draw_circles_funk_t ) noexcept ;

        public: // all rects

            struct rect
            {
                motor::math::vec2f_t points[4] ;
                motor::math::vec4f_t color ;
                motor::math::vec4f_t border ;
            };
            using rect_t = rect ;
            using draw_rects_funk_t = std::function< rect_t ( size_t const i ) > ;

            // draw single rect with border
            void_t draw_rect( size_t const, motor::math::vec2f_cref_t p0, motor::math::vec2f_cref_t p1,
                motor::math::vec2f_cref_t p2, motor::math::vec2f_cref_t p3, motor::math::vec4f_cref_t color,
                motor::math::vec4f_cref_t border_color ) noexcept ;

            // draw multi-rects w/o border
            void_t draw_rects( size_t const l, size_t const num_rects, this_t::draw_rects_funk_t ) noexcept ;
            void_t draw_rects_border( size_t const l, size_t const num_rects, this_t::draw_rects_funk_t ) noexcept ;

        public:

            void_t draw_circle( size_t const, size_t const, motor::math::vec2f_cref_t p0, float_t const r, 
                motor::math::vec4f_cref_t color, motor::math::vec4f_cref_t border_color ) noexcept ;

            void_t set_view_proj( motor::math::mat4f_cref_t view, motor::math::mat4f_cref_t proj ) noexcept ;

        public:

            void_t configure( motor::graphics::gen4::frontend_mtr_t fe ) noexcept ;
            // copy all data to the gpu buffer and transmit the data
            void_t prepare_for_rendering( void_t ) noexcept ;
            void_t prepare_for_rendering( motor::graphics::gen4::frontend_mtr_t ) noexcept ;
            void_t render( motor::graphics::gen4::frontend_mtr_t, size_t const ) noexcept ;
        };
        motor_typedef( primitive_render_2d ) ;
    }
}