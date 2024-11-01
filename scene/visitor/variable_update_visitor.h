
#pragma once

#include "ivisitor.h"

namespace motor
{
    namespace scene
    {
        class MOTOR_SCENE_API variable_update_visitor : public ivisitor
        {
            motor_this_typedefs( variable_update_visitor ) ;
            motor_core_dd_id_fn() ;

        public:

            variable_update_visitor( void_t ) noexcept ;
            virtual ~variable_update_visitor( void_t ) noexcept ;

        public:



            virtual motor::scene::result visit( motor::scene::ivisitable_ptr_t ) noexcept ;
            virtual motor::scene::result post_visit( motor::scene::ivisitable_ptr_t, motor::scene::result const ) noexcept ;
        };
        motor_typedef( variable_update_visitor ) ;
    }
}