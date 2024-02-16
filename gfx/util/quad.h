
#pragma once

#include "../api.h"
#include "../typedefs.h"

#include <motor/graphics/object/render_object.h>
#include <motor/graphics/object/geometry_object.h>
#include <motor/graphics/object/state_object.h>

#include <motor/graphics/frontend/gen4/frontend.hpp>

namespace motor
{
    namespace gfx
    {
        class MOTOR_GFX_API quad
        {
            motor_this_typedefs( quad ) ;

        private:

            motor::string_t _name ;
            
            motor::math::mat4f_t _view = motor::math::mat4f_t().identity() ;
            motor::math::mat4f_t _proj = motor::math::mat4f_t().identity() ;
            motor::math::mat4f_t _world = motor::math::mat4f_t().identity() ;

            motor::graphics::state_object_mtr_t _rs = nullptr ;
            motor::graphics::shader_object_mtr_t _so = nullptr ;
            motor::graphics::render_object_mtr_t _ro = nullptr ;
            motor::graphics::geometry_object_mtr_t _go = nullptr ;

            struct vertex { motor::math::vec3f_t pos ; } ;

        public:

            quad( motor::string_cref_t name ) noexcept ;
            quad( this_cref_t ) = delete ;
            quad( this_rref_t ) noexcept ;
            ~quad( void_t ) noexcept ;

        public:

            void_t init( size_t const vs = 1 ) noexcept ;

        public:
            
            void_t set_view_proj( motor::math::mat4f_cref_t view, motor::math::mat4f_cref_t proj ) noexcept ;

            void_t set_position( motor::math::vec2f_cref_t ) noexcept ;
            void_t set_position( size_t const vs, motor::math::vec2f_cref_t ) noexcept ;
            void_t set_scale( motor::math::vec2f_cref_t ) noexcept ;
            bool_t set_texture( motor::string_cref_t ) noexcept ;
            bool_t set_texture( size_t const i, motor::string_cref_t ) noexcept ;

            bool_t set_texcoord( motor::math::vec4f_cref_t ) noexcept ;
            bool_t set_texcoord( size_t const i, motor::math::vec4f_cref_t ) noexcept ;

            void_t add_variable_sets( size_t const vs ) noexcept ;
            size_t get_num_variable_sets( void_t ) const noexcept ;

        public:

            void_t on_frame_render( motor::graphics::gen4::frontend_mtr_t ) noexcept ;
            void_t on_frame_render( size_t const, motor::graphics::gen4::frontend_mtr_t ) noexcept ;

            void_t on_frame_init( motor::graphics::gen4::frontend_mtr_t ) noexcept ;
            void_t on_frame_release( motor::graphics::gen4::frontend_mtr_t ) noexcept ;
        };
        motor_typedef( quad ) ;
    }
}