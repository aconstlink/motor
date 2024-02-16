
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
        class MOTOR_GFX_API line_render_3d
        {
            motor_this_typedefs( line_render_3d ) ;

        private:

            struct line
            {
                typedef struct
                {
                    motor::math::vec3f_t p0 ;
                    motor::math::vec3f_t p1 ;
                } points_t ;

                union
                {
                    points_t points ;
                    motor::math::vec3f_t pa[2] ;
                };
                
                motor::math::vec4f_t color ;

                line( void_t ) {}
                line( line const & rhv ) 
                {
                    points = rhv.points ;
                    color = rhv.color ;
                }
                line( line && rhv ) noexcept
                {
                    points = std::move( rhv.points ) ;
                    color = std::move( rhv.color ) ;
                }

                line & operator = ( line const & rhv ) noexcept
                {
                    points = rhv.points ;
                    color = rhv.color ;
                    return *this ;
                }
                line & operator = ( line && rhv ) noexcept
                {
                    points = std::move( rhv.points ) ;
                    color = std::move( rhv.color ) ;
                    return *this ;
                }
            };
            motor_typedef( line ) ;

            struct vertex
            {
                motor::math::vec3f_t pos ;
            };

            struct the_data
            {
                motor::math::vec4f_t color ;
            };

        private:

            motor::string_t _name ;
            motor::graphics::async_views_t _asyncs ;
            motor::graphics::state_object_res_t _rs ;
            motor::graphics::array_object_res_t _ao ;
            motor::graphics::shader_object_res_t _so ;
            motor::graphics::render_object_res_t _ro ;
            motor::graphics::geometry_object_res_t _go ;

            motor::math::mat4f_t _proj ;
            motor::math::mat4f_t _view ;

            motor::concurrent::mutex_t _lines_mtx ;
            motor::vector< line_t > _lines ;
            size_t _num_lines = 0 ;

        public:

            line_render_3d( void_t ) noexcept ;
            line_render_3d( this_cref_t ) = delete ;
            line_render_3d( this_rref_t ) noexcept ;
            ~line_render_3d( void_t ) noexcept ;

        public:

            void_t init( motor::string_cref_t, motor::graphics::async_views_t ) noexcept ;
            void_t release( void_t ) noexcept ;

        public: // draw functions

            void_t draw( motor::math::vec3f_cref_t p0, motor::math::vec3f_cref_t p1, motor::math::vec4f_cref_t color ) noexcept ;


            
            

            #if 0 // for later
            void_t draw_cube( motor::math::vec3f_cref_t center, float_t const half, motor::math::vec4f_cref_t color ) noexcept ;
            void_t draw_rect( motor::math::vec3f_cref_t p0, motor::math::vec3f_cref_t p1, 
                motor::math::vec3f_cref_t p2, motor::math::vec3f_cref_t p3, motor::math::vec4f_cref_t color ) noexcept ;

            void_t draw_circle( motor::math::vec2f_cref_t center, float_t const r, size_t const num_points, motor::math::vec4f_cref_t color ) noexcept ;
            
            #endif

        public:

            // copy all data to the gpu buffer and transmit the data
            void_t prepare_for_rendering( void_t ) noexcept ;
            void_t render( void_t ) noexcept ;

            void_t set_view_proj( motor::math::mat4f_cref_t view, motor::math::mat4f_cref_t ) noexcept ;

        private:

            void_t add_variable_set( motor::graphics::render_object_ref_t rc ) noexcept ;

        } ;
        motor_res_typedef( line_render_3d ) ;
    }
}