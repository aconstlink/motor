#pragma once 

#include "node.h"

namespace motor
{
    namespace scene
    {
        class MOTOR_SCENE_API decorator : public node
        {
            motor_typedefs( node, base ) ;
            motor_this_typedefs( decorator ) ;
            motor_core_dd_id_fn() ;

        private:

            node_ptr_t _decorated = nullptr ;

        protected:

            decorator( void_t ) noexcept ;
            decorator( this_rref_t ) noexcept ;
            decorator( this_cref_t ) = delete ;
            decorator( node_ptr_t ) noexcept ;

        public:

            virtual ~decorator( void_t ) noexcept ;

        public:

            virtual motor::scene::result apply( motor::scene::ivisitor_ptr_t ptr ) noexcept ;
            virtual motor::scene::result replace( node_ptr_t which, node_ptr_t with ) noexcept ;
            virtual motor::scene::result detach( node_ptr_t which_ptr ) noexcept ;

        public:

            node_ptr_t borrow_decorated( void_t ) noexcept ;
            node_cptr_t borrow_decorated( void_t ) const noexcept ;

            this_ref_t operator = ( this_cref_t ) = delete ;
            this_ref_t operator = ( this_rref_t ) noexcept ;


        protected:

            void_t set_decorated( node_mtr_safe_t nptr, bool_t const reset_parent ) noexcept ;
            void_t set_decorated( node_mtr_safe_t nptr ) noexcept ;

            motor::scene::result traverse_decorated( motor::scene::ivisitor_ptr_t ptr ) noexcept ;
        };
    }
}

