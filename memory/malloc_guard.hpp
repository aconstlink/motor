#pragma once

#include "typedefs.h"

#include <cstdlib>
#include <cstring>

namespace motor
{
    namespace memory
    {
        /// designed to be used within tiny scopes for
    /// temp. string processing for example.
    /// allocates raw memory.
    /// does not call c/dtors.
    /// frees on destruction.
        template< typename T >
        class malloc_guard
        {
            motor_this_typedefs( malloc_guard<T> ) ;
            motor_typedefs( T, type ) ;

        private: // data

            type_t* _mem_ptr = nullptr ;
            size_t _num_elements = 0 ;

        public:

            malloc_guard( void_t ) noexcept {}

            malloc_guard( size_t const num_elements ) noexcept :
                _mem_ptr( motor::memory::global::alloc_raw<type_t>( sizeof( type_t ) * num_elements ) ), 
                _num_elements( num_elements ) {}

            // creates new memory and copies the const source data to the new memory.
            malloc_guard( type_cptr_t src_ptr, size_t const num_elements ) noexcept :
                _num_elements( num_elements )
            {
                size_t const sib = sizeof( type_t ) * num_elements ;

                _mem_ptr = motor::memory::global::alloc_raw<type_t>( sib ) ;

                std::memcpy( _mem_ptr, src_ptr, sib ) ;
            }

            // takes over the non cost ptr as memory and will delete it during destruction.
            malloc_guard( type_ptr_t src_ptr, size_t const num_elements ) noexcept :
                _mem_ptr( src_ptr ), _num_elements( num_elements ) {}

            malloc_guard( this_rref_t rhv ) noexcept
            {
                *this = std::move( rhv ) ;
            }

            ~malloc_guard( void_t ) noexcept
            {
                motor::memory::global::dealloc_raw( _mem_ptr ) ;
            }

            malloc_guard( this_cref_t ) = delete ;

        public: // operator 

            this_ref_t operator = ( this_rref_t rhv ) noexcept
            {
                //motor_assert( _mem_ptr == nullptr ) ;
                motor_move_member_ptr( _mem_ptr, rhv ) ;
                _num_elements = rhv._num_elements ;
                return *this ;
            }

            type_ref_t operator [] ( size_t i ) noexcept
            {
                //motor_assert( _mem_ptr != nullptr && i < _num_elements ) ;
                return _mem_ptr[ i ] ;
            }

            type_cref_t operator [] ( size_t i ) const noexcept
            {
                //motor_assert( _mem_ptr != nullptr && i < _num_elements ) ;
                return _mem_ptr[ i ] ;
            }

            bool_t equals( size_t i, type_cref_t rhv ) const noexcept
            {
                //motor_assert( _mem_ptr != nullptr && i < _num_elements ) ;
                return _mem_ptr[ i ] == rhv ;
            }

            size_t size( void_t ) const noexcept
            {
                return _num_elements ;
            }

        public:

            type_ptr_t move_ptr( void_t ) noexcept
            {
                _num_elements = 0 ;
                auto ptr = _mem_ptr ;
                _mem_ptr = nullptr ;
                return ptr ;
            }

            type_ptr_t get( void_t ) noexcept { return _mem_ptr ; }
            type_cptr_t get( void_t ) const noexcept { return _mem_ptr ; }

            operator type_ptr_t( void_t ) noexcept { return _mem_ptr ; }
            operator type_cptr_t( void_t ) const noexcept { return _mem_ptr ; }
        };
    }
}
