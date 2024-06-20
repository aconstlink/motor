#pragma once

#include "../group.h"

namespace motor
{
    namespace scene
    {
        /// idea:
        /// activate/deactivate traversal
        class switch_group : public group
        {
            typedef group base_t ;
            motor_this_typedefs( switch_group ) ;

            typedef motor::vector< bool_t > bools_t ;

        private:

            // one-to-one relation to children
            bools_t _actives ;

        public:

            switch_group( void_t ) noexcept ;
            switch_group( node_ptr_t ) noexcept ;
            switch_group( this_rref_t ) noexcept ;
            switch_group( this_cref_t ) = delete ;
            virtual ~switch_group( void_t ) noexcept ;

        public:

            /// if set active, the ith child is added to the base group.
            /// if set deactive, the ith child is removed from the base group.
            motor::scene::result switch_child( size_t const child_i, bool_t const active ) noexcept ;

            /// switches off all children, so no children are traversed anymore.
            motor::scene::result switch_off_all( void_t ) noexcept ;

            /// switches on a particular node.
            motor::scene::result switch_on( node_ptr_t nptr ) noexcept ;


            motor::scene::result add_child( node_mtr_safe_t nptr, bool_t const active ) noexcept ;
            motor::scene::result remove_child( node_ptr_t nptr ) noexcept ;


        public:

            virtual motor::scene::result apply( motor::scene::ivisitor_ptr_t ptr ) noexcept ;
            virtual motor::scene::result detach( node_ptr_t which_ptr ) noexcept ;

        };
    }
}
