#pragma once

#include "../api.h"
#include "../typedefs.h"
#include "../result.h"
#include "../protos.h"

namespace motor
{
    namespace scene
    {
        class MOTOR_SCENE_API node
        {
            motor_this_typedefs( node ) ;

        private:

            /// every node has a parent node
            /// if no parent exists, it is the root
            this_ptr_t _parent = nullptr ;

        public:

            node( void_t ) noexcept ;
            node( this_ptr_t ) noexcept ;
            node( this_rref_t ) noexcept ;
            node( this_cref_t ) = delete ;
            virtual ~node( void_t ) noexcept ;

        public: // parent

            /// returns the parent node.
            this_ptr_t borrow_parent( void_t ) noexcept ;

            /// allows to set the parent of this node while
            /// returning the old one.
            this_ptr_t set_parent( this_ptr_t ) noexcept ;

            //virtual motor::scene::result replace( this_ptr_t which_ptr, this_ptr_t with_ptr ) = 0 ;
            //virtual motor::scene::result detach( this_ptr_t which_ptr ) = 0 ;

        public:

            virtual motor::scene::result apply( motor::scene::ivisitor_ptr_t ) noexcept ;

            this_ref_t operator = ( this_cref_t ) = delete ;
            this_ref_t operator = ( this_rref_t ) noexcept ;

        public:

            template< typename T >
            static bool_t is_of( this_ptr_t ptr ) noexcept
            {
                return dynamic_cast<T*>( ptr ) != nullptr ;
            }

            template< typename T >
            static T * safe_cast( this_ptr_t ptr ) noexcept
            {
                return dynamic_cast< T* >( ptr ) ;
            }

            template< typename T >
            static T * just_cast( this_ptr_t ptr ) noexcept
            {
                return static_cast< T* >( ptr ) ;
            }
        };
        motor_typedef( node ) ;
    }
}
