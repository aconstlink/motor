
#pragma once

#include <motor/base/macros/typedef.h>
#include <cstring>
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

        vector_pod( this_cref_t rhv ) noexcept : _capacity( rhv._capacity ),
            _size( rhv._size ), _ptr( motor::memory::global::alloc_raw<T>( _size ) )
        {
            std::memcpy( _ptr, rhv._ptr, sizeof( T ) * _size ) ;
        }

        vector_pod( this_rref_t rhv ) noexcept : _capacity( rhv._capacity ),
            _size( rhv._size ), _ptr ( motor::move( rhv._ptr ) )
        {
            rhv._capacity = 0 ;
            rhv._size = 0 ;
        }

        this_ref_t operator = ( this_rref_t rhv ) noexcept
        {
            _size = rhv._size ;
            _capacity = rhv._capacity ;

            motor::memory::global::dealloc_raw( _ptr ) ;
            _ptr = motor::move( rhv._ptr ) ;

            rhv._capacity = 0 ;
            rhv._size = 0 ;
            return *this ;
        }

        this_ref_t operator = ( this_cref_t rhv ) noexcept
        {
            _size = rhv._size ;
            _capacity = rhv._capacity ;

            motor::memory::global::dealloc_raw( _ptr ) ;
            _ptr = motor::memory::global::alloc_raw<T>( _size  ) ;
            std::memcpy( _ptr, rhv._ptr, sizeof( T ) * _size ) ;
            
            return *this ;
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

        void_t reseve( size_t const num_elems, size_t const grow = 10 ) noexcept
        {
            if ( _capacity > num_elems ) return ;

            _capacity = num_elems + grow ;

            auto * new_mem = motor::memory::global_t::alloc_raw<T>( _capacity ) ;
            if ( _ptr != nullptr )
            {
                std::memcpy( new_mem, _ptr, sizeof( T ) * _size ) ;
            }
            motor::memory::global::dealloc_raw( _ptr ) ;
            _ptr = new_mem ;
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

        void_t push_back( T const & v, size_t const grow_by = 10 ) noexcept
        {
            this_t::reseve( _capacity + 1, grow_by ) ;
            _ptr[ _size++ ] = v ;
        }
    };


} 
template< typename T >
using vector_pod = mstd::vector_pod<T> ;
}