#pragma once

#include "ivisitor.h"
#include <motor/graphics/frontend/gen4/frontend.hpp>

namespace motor
{
    namespace scene
    {
        class MOTOR_SCENE_API render_visitor : public ivisitor
        {
            motor_core_dd_id_fn() ;
            motor_this_typedefs( render_visitor ) ;

            motor::graphics::gen4::frontend_ptr_t _fe ;

        public:

            render_visitor( motor::graphics::gen4::frontend_ptr_t ) noexcept ;
            render_visitor( this_rref_t ) noexcept ;
            render_visitor( this_cref_t ) = delete ;
            virtual ~render_visitor( void_t ) noexcept ;

        public:

            virtual motor::scene::result visit( motor::scene::render_node_ptr_t ) noexcept  ;

            virtual motor::scene::result visit( motor::scene::render_settings_ptr_t ) noexcept  ;
            virtual motor::scene::result post_visit( motor::scene::render_settings_ptr_t, motor::scene::result const ) noexcept ;
            
            virtual motor::scene::result visit( motor::scene::trafo3d_node_ptr_t ) noexcept  ;
            virtual motor::scene::result post_visit( motor::scene::trafo3d_node_ptr_t, motor::scene::result const ) noexcept ;

            

            virtual void_t on_start( void_t ) noexcept ;
            virtual void_t on_finish( void_t ) noexcept ;
        };
        motor_typedef( render_visitor ) ;
    }
}