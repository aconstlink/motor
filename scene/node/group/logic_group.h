#pragma once

#include "../group.h"

namespace motor
{
    namespace scene
    {
        class MOTOR_SCENE_API logic_group : public group
        {
            typedef group base_t ;

            motor_this_typedefs( logic_group ) ;

        public:

            logic_group( void_t ) noexcept ;
            logic_group( node_ptr_t ) noexcept ;
            logic_group( this_rref_t ) noexcept ;
            logic_group( this_cref_t ) = delete ;
            virtual ~logic_group( void_t ) noexcept  ;

        public:

            motor::scene::result add_child( node_mtr_safe_t nptr ) noexcept ;
            size_t remove_child( node_ptr_t nptr ) noexcept ;

        public:

            virtual motor::scene::result apply( motor::scene::ivisitor_ptr_t ptr ) noexcept ;

        };
        motor_typedef( logic_group ) ;
    }
}