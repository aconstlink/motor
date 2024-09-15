#pragma once

#include "ivisitor.h"

namespace motor
{
    namespace scene
    {
        class MOTOR_SCENE_API render_visitor : public ivisitor
        {
        public:

            virtual motor::scene::result visit( motor::scene::node_ptr_t ) noexcept ;
            virtual motor::scene::result visit( motor::scene::leaf_ptr_t ) noexcept ;

            virtual motor::scene::result visit( motor::scene::group_ptr_t ) noexcept ;
            virtual motor::scene::result post_visit( motor::scene::group_ptr_t, motor::scene::result const ) noexcept ;
            virtual motor::scene::result visit( motor::scene::camera_node_ptr_t ) noexcept ;
            virtual motor::scene::result visit( motor::scene::trafo3d_node_ptr_t ) noexcept  ;
            virtual motor::scene::result post_visit( motor::scene::trafo3d_node_ptr_t, motor::scene::result const ) noexcept ;

            virtual void_t on_start( void_t ) noexcept ;
            virtual void_t on_finish( void_t ) noexcept ;
        };
        motor_typedef( render_visitor ) ;
    }
}