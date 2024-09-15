#pragma once

#include "../api.h"
#include "../typedefs.h"
#include "../result.h"
#include "../protos.h"

#include <motor/std/vector>

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

            motor::vector< motor::scene::icomponent_mtr_t > _components ;

        public:

            node( void_t ) noexcept ;
            node( this_ptr_t ) noexcept ;
            node( this_rref_t ) noexcept ;
            node( this_cref_t ) = delete ;
            virtual ~node( void_t ) noexcept ;

            this_ref_t operator = ( this_cref_t ) = delete ;
            this_ref_t operator = ( this_rref_t ) noexcept ;

        public: // parent

            /// returns the parent node.
            this_ptr_t borrow_parent( void_t ) noexcept ;

            /// allows to set the parent of this node while
            /// returning the old one.
            this_ptr_t set_parent( this_ptr_t ) noexcept ;

            //virtual motor::scene::result replace( this_ptr_t which_ptr, this_ptr_t with_ptr ) = 0 ;
            //virtual motor::scene::result detach( this_ptr_t which_ptr ) = 0 ;

        public: // component 

            template< typename T >
            T * borrow_component( void_t ) const noexcept
            {
                for ( auto * comp : _components )
                {
                    if ( auto * ret = dynamic_cast<T*>( comp ); ret != nullptr )
                    {
                        return ret ;
                    }
                }
                return nullptr ;
            }

            template< typename T >
            bool_t borrow_component( T * & ret ) const noexcept
            {
                for ( auto * comp : _components )
                {
                    if ( ret = dynamic_cast<T *>( comp ); ret != nullptr )
                    {
                        return true ;
                    }
                }
                return false ;
            }

            template< typename T >
            bool_t add_component( motor::core::mtr_safe<T> comp ) noexcept
            {
                auto iter = std::find_if( _components.begin(), _components.end(), 
                [&] ( motor::scene::icomponent_ptr_t comp_in ) 
                {
                    return comp_in == comp || (dynamic_cast<T *>( comp_in ) != nullptr) ;
                } ) ;

                if ( iter != _components.end() ) 
                {
                    motor::release( comp ) ;
                    return false ;
                }

                _components.emplace_back( motor::move( comp ) ) ;

                return true ;
            }

            using for_each_comp_funk_t = std::function< void_t ( motor::scene::icomponent_ptr_t ) > ;
            void_t for_each_component( for_each_comp_funk_t funk ) noexcept 
            {
                for( auto * comp : _components )
                {
                    funk( comp ) ;
                }
            }

        public:

            // helps traversing the tree
            // use this call to access the apply() function.
            class MOTOR_SCENE_API traverser
            {
            private:

                node_ptr_t _traverse ;

            public:

                traverser( node_ptr_t begin ) noexcept ;
                void_t apply( motor::scene::ivisitor_ptr_t ) noexcept ;
            };

            // for nodes to be called
            class derived_apply
            {
            private:

                node_ptr_t _traverse ;

            public:

                derived_apply( node_ptr_t begin ) noexcept ;
                motor::scene::result apply( motor::scene::ivisitor_ptr_t ) noexcept ;
            };

        protected:

            virtual motor::scene::result apply( motor::scene::ivisitor_ptr_t ) noexcept ;

            

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
