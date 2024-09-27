#pragma once

#include "node.h"

namespace motor
{
    namespace scene
    {
        class MOTOR_SCENE_API leaf : public node
        {
            typedef node base_t ;
            motor_this_typedefs( leaf ) ;
            motor_core_dd_id_fn() ;

        public:

            leaf( void_t ) noexcept ;
            leaf( node_ptr_t ) noexcept ;
            leaf( this_rref_t ) noexcept ;
            virtual ~leaf( void_t ) noexcept ;

        public:

            virtual motor::scene::result apply( motor::scene::ivisitor_ptr_t ptr ) noexcept ;
        };
        motor_typedef( leaf ) ;
    }
}