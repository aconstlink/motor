#pragma once

#include "ivisitor.h"

namespace motor
{
    namespace scene
    {
        class MOTOR_SCENE_API trafo_visitor : public ivisitor
        {
        public:

            trafo_visitor( void_t ) noexcept ;
            virtual ~trafo_visitor( void_t ) noexcept {}

            virtual motor::scene::result visit( motor::scene::node_ptr_t ) noexcept = 0 ;
            virtual motor::scene::result visit( motor::scene::leaf_ptr_t ) noexcept = 0 ;

            virtual motor::scene::result visit( motor::scene::group_ptr_t ) noexcept = 0 ;
            virtual motor::scene::result post_visit( motor::scene::group_ptr_t, motor::scene::result const ) noexcept = 0 ;

            virtual void_t on_start( void_t ) noexcept {}
            virtual void_t on_finish( void_t ) noexcept {}
        };
    }
}