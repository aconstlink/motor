
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

        vector_pod( std::initializer_list< T > && l ) noexcept
        {
            this_t::resize( l.size() ) ;

            size_t i=size_t(-1); 
            for( auto && v : l ) 
            {
                _ptr[++i] = std::move( v ) ;
            }
        }

        ~vector_pod( void_t ) noexcept
        {
            motor::memory::global::dealloc_raw( _ptr ) ;
        }

        vector_pod( size_t const num_elems ) noexcept
        {
            this_t::resize( num_elems ) ;
        }

        vector_pod( this_cref_t rhv ) noexcept : _capacity( rhv._capacity ),
            _size( rhv._size ), _ptr( nullptr )
        {
            _ptr = motor::memory::global::alloc_raw<T>( _capacity ) ;
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

        bool_t contains( T const & v ) const noexcept
        {
            return this_t::find( v ) != _size ;
        }

        size_t find( T const & v ) const noexcept
        {
            size_t i = size_t(-1) ;
            while ( ++i < _size && _ptr[ i ] != v ) ;
            return i ;
        }

        // erases one element v
        void_t erase( T const & v ) noexcept
        {
            return this_t::erase_by_index( this_t::find( v ) ) ;
        }

        // erase the ith entry
        void_t erase_by_index( size_t const i ) noexcept
        {
            if( i >= _size ) return ;

            size_t const src = i ;
            size_t const dst = i + 1 ;

            std::memcpy( _ptr + src, _ptr + dst, sizeof( T ) * (_size - dst) ) ;

            _size -= 1 ;
        }

        void_t insert( size_t const idx, T const & item ) noexcept
        {
            if( idx == _size ) return this_t::push_back( item ) ;

            this_t::make_room( idx, 1 ) ;
            _ptr[idx] = item ;
        }

        void_t insert( size_t const idx, T && item ) noexcept
        {
            if( idx == _size ) return this_t::push_back( std::move( item ) ) ;

            this_t::make_room( idx, 1 ) ;
            _ptr[idx] = std::move( item ) ;
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

        T const & front( void_t ) const noexcept
        {
            assert( _size > 0 ) ;
            return _ptr[0] ;
        }

        T const & back( void_t ) const noexcept
        {
            assert( _size != 0 ) ;
            return _ptr[ _size - 1 ] ;
        }

        void_t reserve( size_t const num_elems, size_t const grow = 10 ) noexcept
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

        size_t resize_and_init( size_t const num_elems, size_t const grow = 10 ) noexcept
        {
            auto old_size = this_t::resize( num_elems, grow ) ;
            if( old_size >= num_elems ) return old_size ;

            for( size_t i=old_size; i<num_elems; ++i )
            {
                *new(_ptr+i)T() ;
            }
            return old_size ;
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

        void_t push_back( T const & v, size_t const grow_by = 10 ) noexcept
        {
            size_t const pos = this_t::resize_by( 1, grow_by ) ;
            _ptr[ pos ] = v ;
        }

        void_t push_back( T && v, size_t const grow_by = 10 ) noexcept
        {
            size_t const pos = this_t::resize_by( 1 ) ;
            _ptr[ pos ] = std::move( v ) ;
        }

    public:

        T const & operator [] ( size_t const i ) const noexcept
        {
            assert( i < _capacity ) ;
            return _ptr[ i ] ;
        }

        T & operator [] ( size_t const i ) noexcept
        {
            assert( i < _size ) ;
            return _ptr[ i ] ;
        }

        T * data( void_t ) noexcept
        {
            return _ptr ;
        }

        T const * data( void_t ) const noexcept
        {
            return _ptr ;
        }

    public: // for each

        void_t for_each( std::function< void_t ( size_t const i, T const & v ) > f ) const noexcept
        {
            for ( size_t i = 0; i < _size; ++i ) f( i, _ptr[ i ] ) ;
        }
        
        void_t for_each( std::function< void_t ( size_t const, T & ) > f ) noexcept
        {
            for ( size_t i = 0; i < _size; ++i ) f( i, _ptr[ i ] ) ;
        }

    private:

        // shifts the memory to the right by num_elems
        // also resizes the internal data by num_elems
        // this makes room at idx by num_elems
        // @note the new space is uninitialized or contains old values!
        void_t make_room( size_t const idx, size_t const num_elems ) noexcept
        {
            size_t const old_size = this_t::resize_by( num_elems ) ;
            size_t const dif = old_size - idx ;
            std::memcpy( void_ptr_t( _ptr + idx + num_elems), void_cptr_t( _ptr + idx ), sizeof(T) * dif ) ;
        }

        // makes what make_room does but also zeros the memory.
        void_t make_room_and_zero( size_t const idx, size_t const num_elems ) noexcept
        {
            this_t::make_room( idx, num_elems ) ;
            std::memset( void_ptr_t( _ptr + idx ), 0, sizeof(T) * num_elems ) ;
        }
    };


} 
template< typename T >
using vector_pod = mstd::vector_pod<T> ;
}