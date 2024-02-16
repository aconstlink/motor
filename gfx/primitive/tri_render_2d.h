#pragma once

#include "../api.h"
#include "../typedefs.h"

#include <motor/graphics/async.h>
#include <motor/graphics/object/state_object.h>
#include <motor/graphics/object/array_object.h>
#include <motor/graphics/object/render_object.h>
#include <motor/graphics/object/geometry_object.h>
#include <motor/graphics/variable/variable_set.hpp>

#include <motor/std/vector>

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
                struct points
                {
                    motor::math::vec2f_t p0 ;
                    motor::math::vec2f_t p1 ;
                    motor::math::vec2f_t p2 ;
                };

                union 
                {
                    points pts ;
                    motor::math::vec2f_t array[3] ;
                };
                motor::math::vec4f_t color ;

                tri( void_t ) {}
                tri( tri const & rhv ) 
                {
                    std::memcpy( reinterpret_cast<void_ptr_t>(this),
                        reinterpret_cast<void_cptr_t>(&rhv), sizeof(tri) ) ;
                }
                tri( tri && rhv ) noexcept
                {
                    std::memcpy( reinterpret_cast<void_ptr_t>(this),
                        reinterpret_cast<void_cptr_t>(&rhv), sizeof(tri) ) ;
                }

                tri & operator = ( tri const & rhv ) noexcept
                {
                    std::memcpy( reinterpret_cast<void_ptr_t>(this),
                        reinterpret_cast<void_cptr_t>(&rhv), sizeof(tri) ) ;
                    return *this ;
                }
                tri & operator = ( tri && rhv ) noexcept
                {
                    std::memcpy( reinterpret_cast<void_ptr_t>(this),
                        reinterpret_cast<void_cptr_t>(&rhv), sizeof(tri) ) ;
                    return *this ;
                }
            };
            motor_typedef( tri ) ;

            struct layer
            {
                motor::concurrent::mutex_t mtx ;
                motor::vector< tri_t > tris ;

                layer( void_t ) noexcept {}
                layer( layer const & rhv ) noexcept { tris = rhv.tris ; }
                layer( layer && rhv ) noexcept { tris = std::move(rhv.tris) ; }
                ~layer( void_t ) noexcept {}
            };
            motor_typedef( layer ) ;

            motor::concurrent::mutex_t _layers_mtx ;
            motor::vector< layer_t > _layers ;

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
            motor::graphics::async_views_t _asyncs ;
            motor::graphics::state_object_res_t _rs ;
            motor::graphics::array_object_res_t _ao ;
            motor::graphics::shader_object_res_t _so ;
            motor::graphics::render_object_res_t _ro ;
            motor::graphics::geometry_object_res_t _go ;

            motor::math::mat4f_t _proj ;
            motor::math::mat4f_t _view ;

        public:

            tri_render_2d( void_t ) ;
            tri_render_2d( this_cref_t ) = delete ;
            tri_render_2d( this_rref_t ) ;
            ~tri_render_2d( void_t ) ;

            void_t init( motor::string_cref_t, motor::graphics::async_views_t ) noexcept ;
            void_t release( void_t ) noexcept ;

        public:

            void_t draw( size_t const, motor::math::vec2f_cref_t p0, motor::math::vec2f_cref_t p1, 
                motor::math::vec2f_cref_t p2, motor::math::vec4f_cref_t color ) noexcept ;

            void_t draw_rect( size_t const, motor::math::vec2f_cref_t p0, motor::math::vec2f_cref_t p1, 
                motor::math::vec2f_cref_t p2, motor::math::vec2f_cref_t p3, motor::math::vec4f_cref_t color ) noexcept ;

            void_t draw_circle( size_t const, size_t const, motor::math::vec2f_cref_t p0, float_t const r, motor::math::vec4f_cref_t color ) noexcept ;

        public:

            // copy all data to the gpu buffer and transmit the data
            void_t prepare_for_rendering( void_t ) noexcept ;
            void_t render( size_t const ) noexcept ;

            void_t set_view_proj( motor::math::mat4f_cref_t view, motor::math::mat4f_cref_t ) noexcept ;

        private:

            motor_typedefs( motor::vector< motor::math::vec2f_t >, circle ) ;
            motor::vector< circle_t > _circle_cache ;
            circle_cref_t lookup_circle_cache( size_t const ) noexcept ;

            void_t add_variable_set( motor::graphics::render_object_ref_t rc ) noexcept ;

            bool_t has_data_for_layer( size_t const l ) const noexcept ;
        };
        motor_res_typedef( tri_render_2d ) ;
    }
}