#pragma once

#include "../api.h"
#include "../typedefs.h"

#include <motor/graphics/object/state_object.h>
#include <motor/graphics/object/array_object.h>
#include <motor/graphics/object/render_object.h>
#include <motor/graphics/object/geometry_object.h>
#include <motor/graphics/variable/variable_set.hpp>
#include <motor/graphics/frontend/gen4/frontend.hpp>

#include <motor/concurrent/mrsw.hpp>

#include <motor/std/vector>
#include <motor/std/vector_pod>

namespace motor
{
    namespace gfx
    {
        class MOTOR_GFX_API line_render_2d
        {
            motor_this_typedefs( line_render_2d ) ;

        public: // data

            struct line
            {
                motor::math::vec2f_t points[2] ;
                motor::math::vec4f_t color ;
            };
            motor_typedef( line ) ;

        private:

            struct layer
            {
                motor::concurrent::mrsw_t mtx ;
                motor::vector_pod< line_t > lines ;
            };
            motor_typedef( layer ) ;

            motor::concurrent::mutex_t _layers_mtx ;
            motor::vector< layer_ptr_t > _layers ;

            motor::concurrent::mutex_t _num_lines_mtx ;
            size_t _num_lines = 0 ;

        private: // graphics

            struct per_layer_render_data
            {
                size_t start = 0 ;
                size_t num_elems = 0 ;
            };
            motor::vector< per_layer_render_data > _render_data ;

            struct vertex
            {
                motor::math::vec2f_t pos ;
            };

            struct the_data
            {
                motor::math::vec4f_t color ;
            };

            motor::string_t _name ;
            motor::graphics::state_object_t _rs ;
            motor::graphics::array_object_t _ao ;
            motor::graphics::shader_object_t _so ;
            motor::graphics::render_object_t _ro ;
            motor::graphics::geometry_object_t _go ;

            motor::math::mat4f_t _proj ;
            motor::math::mat4f_t _view ;

            struct prepare_update
            {
                bool_t vertex_realloc = false ;
                bool_t data_realloc = false ;
                bool_t reconfig_ro = false ;
            };

            prepare_update _pe ;

        public:

            line_render_2d( void_t ) noexcept ;
            line_render_2d( this_cref_t ) = delete ;
            line_render_2d( this_rref_t ) noexcept ;
            ~line_render_2d( void_t ) noexcept ;

            void_t init( motor::string_cref_t ) noexcept ;
            void_t release( void_t ) noexcept ;

            this_ref_t operator = ( this_rref_t ) noexcept ;

        public:

            void_t draw( size_t const, motor::math::vec2f_cref_t p0, motor::math::vec2f_cref_t p1, 
                motor::math::vec4f_cref_t color ) noexcept ;

            void_t draw_rect( size_t const, motor::math::vec2f_cref_t p0, motor::math::vec2f_cref_t p1, 
                motor::math::vec2f_cref_t p2, motor::math::vec2f_cref_t p3, motor::math::vec4f_cref_t color ) noexcept ;

            void_t draw_circle( size_t const layer, size_t const num_points, motor::math::vec2f_cref_t p0, float_t const r, motor::math::vec4f_cref_t color ) noexcept ;

        public: // multi-draw

            using draw_lines_funk_t = std::function< line_t ( size_t const ) > ;
            void_t draw_lines( size_t const layer, size_t const num_lines, draw_lines_funk_t ) noexcept ;

            struct rect
            {
                motor::math::vec2f_t points[4] ;
                motor::math::vec4f_t color ;
            };
            using rect_t = rect ;
            using draw_rects_funk_t = std::function< rect_t ( size_t const ) > ;
            void_t draw_rects( size_t const layer, size_t const num_rects, draw_rects_funk_t ) noexcept ;


            struct circle
            {
                motor::math::vec2f_t pos ;
                float_t radius ;
                motor::math::vec4f_t color ;
            };
            using circle_t = circle ;
            using draw_circles_funk_t = std::function< circle_t ( size_t const ) > ;
            void_t draw_circles( size_t const layer, size_t const num_segs, size_t const num_circles, this_t::draw_circles_funk_t ) noexcept ;

        public:

            void_t configure( motor::graphics::gen4::frontend_mtr_t fe ) noexcept ;
            // copy all data to the gpu buffer and transmit the data
            void_t prepare_for_rendering( void_t ) noexcept ;

            void_t prepare_for_rendering( motor::graphics::gen4::frontend_mtr_t fe ) noexcept ;
            void_t render( motor::graphics::gen4::frontend_mtr_t fe, size_t const ) noexcept ;

            void_t set_view_proj( motor::math::mat4f_cref_t view, motor::math::mat4f_cref_t ) noexcept ;

        private:

            motor_typedefs( motor::vector< motor::math::vec2f_t >, circle_points ) ;
            motor::vector< circle_points_t > _circle_cache ;
            circle_points_cref_t lookup_circle_cache( size_t const ) noexcept ;

            void_t add_variable_set( motor::graphics::render_object_ref_t rc ) noexcept ;

            bool_t has_data_for_layer( size_t const l ) const noexcept ;

            layer_ptr_t add_layer( size_t const i ) noexcept ;
        };
        motor_typedef( line_render_2d ) ;
    }
}