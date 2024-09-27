#pragma once

#include "node.h"
#include <motor/std/vector>

namespace motor
{
    namespace scene
    {
        class MOTOR_SCENE_API group : public node
        {
            typedef node base_t ;
            motor_this_typedefs( group ) ;
            motor_core_dd_id_fn() ;

            typedef motor::vector< node_mtr_t > children_t ;
            typedef std::function< bool_t( size_t i ) > traverse_predicate_t ;

        private:
            
            traverse_predicate_t _funk = [=]( size_t ){ return true ; } ;
            children_t _children ;

        protected:

            /// allows a derived class to add a child node.
            motor::scene::result add_child( node_mtr_safe_t ) noexcept ;

            /// allows a derived class to remove a child node.
            /// the derived class might want to know the index at
            /// which the child was removed, so the index at
            /// which the child was removed will be returned.
            size_t remove_child( node_ptr_t ) noexcept ;

            /// allows to traverse all child nodes. the predicate
            /// function allows to fine grain traversal for certain 
            /// derived class behavior.
            void_t traverse_children( motor::scene::ivisitor_ptr_t ptr,
                traverse_predicate_t func ) noexcept ;

            void_t traverse_children( motor::scene::ivisitor_ptr_t ptr ) noexcept ;

        protected:

            group( void_t ) noexcept ;
            group( node_ptr_t ) noexcept ;
            group( this_rref_t ) noexcept ;
            group( this_cref_t ) = delete ;

        public:

            virtual ~group( void_t ) noexcept ;

        public:

            virtual motor::scene::result apply( motor::scene::ivisitor_ptr_t ) noexcept ;

            /// replaces the node which_ptr with with_ptr.
            /// if which_ptr does not exist, nothing will happen.
            virtual motor::scene::result replace( node_ptr_t which_ptr, node_ptr_t with_ptr ) noexcept ;
            virtual motor::scene::result detach( node_ptr_t which_ptr ) noexcept ;

        public:

            size_t get_num_children( void_t ) const noexcept ;
            node_ptr_t borrow_child( size_t const child_i ) noexcept;

            /// find the index of a given child ptr.
            /// -1 is returned if the passed pointer is not
            /// found in the children container.
            size_t find_index( node_ptr_t ) const noexcept ;
        };
        motor_typedef( group ) ;
    }    
}