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

#include <array>

namespace motor
{
    namespace gfx
    {
        class MOTOR_GFX_API tri_render_2d
        {
            motor_this_typedefs( tri_render_2d ) ;

        private: // data

            struct tri
            {
                motor::math::vec2f_t points[ 3 ] ;
                motor::math::vec4f_t color ;
            };
            motor_typedef( tri ) ;

            struct layer
            {
                motor::concurrent::mrsw_t mtx ;
                motor::vector< tri_t > tris ;

                layer( void_t ) noexcept {}
                layer( layer const & rhv ) noexcept { tris = rhv.tris ; }
                layer( layer && rhv ) noexcept { tris = std::move(rhv.tris) ; }
                ~layer( void_t ) noexcept {}
            };
            motor_typedef( layer ) ;

            motor::concurrent::mutex_t _layers_mtx ;
            motor::vector< layer_ptr_t > _layers ;

            motor::concurrent::mutex_t _num_tris_mtx ;
            size_t _num_tris = 0 ;

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

            tri_render_2d( void_t ) noexcept ;
            tri_render_2d( this_cref_t ) = delete ;
            tri_render_2d( this_rref_t ) noexcept ;
            ~tri_render_2d( void_t ) noexcept ;

            this_ref_t operator = ( this_rref_t ) noexcept ;

            void_t init( motor::string_cref_t ) noexcept ;
            void_t release( void_t ) noexcept ;

        public: // single-draw

            void_t draw( size_t const, motor::math::vec2f_cref_t p0, motor::math::vec2f_cref_t p1, 
                motor::math::vec2f_cref_t p2, motor::math::vec4f_cref_t color ) noexcept ;

            void_t draw_rect( size_t const, motor::math::vec2f_cref_t p0, motor::math::vec2f_cref_t p1, 
                motor::math::vec2f_cref_t p2, motor::math::vec2f_cref_t p3, motor::math::vec4f_cref_t color ) noexcept ;

            void_t draw_circle( size_t const, size_t const, motor::math::vec2f_cref_t p0, float_t const r, motor::math::vec4f_cref_t color ) noexcept ;

        public: // multi-draw (md)

            struct circle_md
            {
                motor::math::vec2f_t pos ;
                float_t radius ;
                motor::math::vec4f_t color ;
            };
            using circle_md_t = circle_md ;

            using draw_circles_funk_t = std::function< circle_md_t ( size_t const ) > ;
            void_t draw_circles( size_t const layer, size_t const segs, size_t const num_circles, draw_circles_funk_t ) noexcept ;

            struct tri_md
            {
                motor::math::vec2f_t points[3] ;
                motor::math::vec4f_t color ;
            };

            using tri_md_t = tri_md ;

            using draw_tris_funk_t = std::function< tri_md_t ( size_t const ) > ;
            void_t draw_tris( size_t const layer, size_t const num_tris, draw_tris_funk_t ) noexcept ;

            struct rect
            {
                motor::math::vec2f_t points[4] ;
                motor::math::vec4f_t color ;
            };
            using rect_md_t = rect ;

            using draw_rects_funk_t = std::function< rect_md_t ( size_t const ) > ;
            void_t draw_rects( size_t const layer, size_t const num_rects, draw_rects_funk_t ) noexcept ;

        public:

            void_t configure( motor::graphics::gen4::frontend_mtr_t fe ) noexcept ;
            // copy all data to the gpu buffer and transmit the data
            void_t prepare_for_rendering( void_t ) noexcept ;
            void_t prepare_for_rendering( motor::graphics::gen4::frontend_mtr_t fe ) noexcept ;
            void_t render( motor::graphics::gen4::frontend_mtr_t fe, size_t const ) noexcept ;

            void_t set_view_proj( motor::math::mat4f_cref_t view, motor::math::mat4f_cref_t ) noexcept ;

        private:

            motor_typedefs( motor::vector< motor::math::vec2f_t >, circle ) ;
            motor::vector< circle_t > _circle_cache ;
            circle_cref_t lookup_circle_cache( size_t const ) noexcept ;

            void_t add_variable_set( motor::graphics::render_object_ref_t rc ) noexcept ;

            bool_t has_data_for_layer( size_t const l ) const noexcept ;

            layer_ptr_t add_layer( size_t const i ) noexcept ;
        };
        motor_typedef( tri_render_2d ) ;
    }
}