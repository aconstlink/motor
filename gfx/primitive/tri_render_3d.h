#pragma once

#include "../api.h"
#include "../typedefs.h"

#include <motor/graphics/object/state_object.h>
#include <motor/graphics/object/array_object.h>
#include <motor/graphics/object/render_object.h>
#include <motor/graphics/object/geometry_object.h>
#include <motor/graphics/variable/variable_set.hpp>
#include <motor/graphics/frontend/gen4/frontend.hpp>

#include <motor/concurrent/typedefs.h>
#include <motor/std/vector>

namespace motor
{
    namespace gfx
    {
        class MOTOR_GFX_API tri_render_3d
        {
            motor_this_typedefs( tri_render_3d ) ;

        private: // data

            struct tri
            {
                struct points
                {
                    motor::math::vec3f_t p0 ;
                    motor::math::vec3f_t p1 ;
                    motor::math::vec3f_t p2 ;
                };

                union 
                {
                    points pts ;
                    motor::math::vec3f_t array[3] ;
                };
                motor::math::vec4f_t color ;

                tri( void_t ) noexcept {}
                tri( tri const & rhv ) noexcept
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

            motor::concurrent::mutex_t _tris_mtx ;
            motor::vector< tri_t > _tris ;
            size_t _num_tris = 0 ;

        private: // graphics

            struct vertex
            {
                motor::math::vec3f_t pos ;
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
            };

            prepare_update _pe ;

        public:

            tri_render_3d( void_t ) noexcept ;
            tri_render_3d( this_cref_t ) = delete ;
            tri_render_3d( this_rref_t ) noexcept ;
            ~tri_render_3d( void_t ) noexcept ;

            this_ref_t operator = ( this_rref_t ) noexcept ;

            void_t init( motor::string_cref_t ) noexcept ;
            void_t release( void_t ) noexcept ;

        public:

            void_t draw( motor::math::vec3f_cref_t p0, motor::math::vec3f_cref_t p1, 
                motor::math::vec3f_cref_t p2, motor::math::vec4f_cref_t color ) noexcept ;

            void_t draw_rect( motor::math::vec3f_cref_t p0, motor::math::vec3f_cref_t p1, 
                motor::math::vec3f_cref_t p2, motor::math::vec3f_cref_t p3, motor::math::vec4f_cref_t color ) noexcept ;

            void_t draw_circle( motor::math::mat3f_cref_t, motor::math::vec3f_cref_t p0, float_t const r, motor::math::vec4f_cref_t color, size_t const ) noexcept ;

        public:

            void_t configure( motor::graphics::gen4::frontend_mtr_t fe ) noexcept ;

            // copy all data to the gpu buffer and transmit the data
            void_t prepare_for_rendering( void_t ) noexcept ;
            void_t prepare_for_rendering( motor::graphics::gen4::frontend_mtr_t fe ) noexcept ;
            void_t render( motor::graphics::gen4::frontend_mtr_t fe ) noexcept ;

            void_t set_view_proj( motor::math::mat4f_cref_t view, motor::math::mat4f_cref_t ) noexcept ;

        private:

            motor_typedefs( motor::vector< motor::math::vec3f_t >, circle ) ;
            motor::vector< circle_t > _circle_cache ;
            circle_cref_t lookup_circle_cache( size_t const ) noexcept ;

            void_t add_variable_set( motor::graphics::render_object_ref_t rc ) noexcept ;

        };
        motor_typedef( tri_render_3d ) ;
    }
}