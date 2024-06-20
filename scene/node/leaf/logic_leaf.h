#pragma once

#include "../leaf.h"

namespace motor
{
    namespace scene
    {
        class MOTOR_SCENE_API logic_leaf : public leaf
        {
            typedef leaf base_t ;
            motor_this_typedefs( logic_leaf ) ;

        public:

            logic_leaf( void_t ) noexcept ;
            logic_leaf( node_ptr_t ) noexcept ;
            logic_leaf( this_rref_t ) noexcept ;
            logic_leaf( this_cref_t ) = delete ;
            virtual ~logic_leaf( void_t ) noexcept ;

        public:

            virtual motor::scene::result apply( motor::scene::ivisitor_ptr_t ) noexcept ;
        };
        motor_typedef( logic_leaf ) ;
    }
}