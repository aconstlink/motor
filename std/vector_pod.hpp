
#pragma once

#include <motor/core/macros/typedef.h>

namespace motor{ namespace mstd {

    template< typename T >
    class vector_pod
    {
        motor_this_typedefs( vector_pod< T > ) ;

        size_t _capacity = 0 ;
        size_t _size = 0 ;

        T * _ptr = nullptr ;

    public:

        vector_pod( void_t ) noexcept {}

        ~vector_pod( void_t ) noexcept
        {
            motor::memory::global::dealloc_raw( _ptr ) ;
        }

        vector_pod( size_t const num_elems ) noexcept
        {
            this_t::resize( num_elems ) ;
        }

        vector_pod( this_cref_t rhv ) noexcept
        {
            _capacity = rhv._capacity ;
            _size = rhv._size ;
        }

        vector_pod( this_rref_t rhv ) noexcept
        {
            _capacity = rhv._capacity ;
            _size = rhv._size ;

            rhv._capacity = 0 ;
            rhv._size = 0 ;
        }

        size_t size( void_t ) const noexcept
        {
            return _size ;
        }

        size_t capacity( void_t ) const noexcept
        {
            return _capacity ;
        }

        void_t clear( void_t ) noexcept
        {
            _size = 0 ;
        }

        size_t resize( size_t const num_elems, size_t const grow = 10 ) noexcept
        {
            size_t const ret = _size ;

            if ( _capacity < num_elems )
            {
                _capacity = num_elems + grow ;

                auto * new_mem = motor::memory::global_t::alloc_raw<T>( _capacity ) ;
                if ( _ptr != nullptr )
                {
                    std::memcpy( new_mem, _ptr, sizeof( T ) * _size ) ;
                }
                motor::memory::global::dealloc_raw( _ptr ) ;
                _ptr = new_mem ;
            }
            _size = num_elems ;

            return ret ;
        }

        size_t resize_by( size_t const num_elems, size_t const grow = 10 ) noexcept
        {
            return this_t::resize( _size + num_elems, grow ) ;
        }

        T const & operator [] ( size_t const i ) const noexcept
        {
            assert( i < _capacity ) ;
            return _ptr[ i ] ;
        }

        T & operator [] ( size_t const i ) noexcept
        {
            assert( i < _capacity ) ;
            return _ptr[ i ] ;
        }
    };


} 
template< typename T >
using vector_pod = mstd::vector_pod<T> ;
}