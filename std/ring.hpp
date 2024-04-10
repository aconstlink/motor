

#pragma once

#include "vector"

namespace motor{ namespace mstd{ 

    template< typename T, size_t num_entries >
    class ring_buffer
    {
        size_t _cur_pos = size_t( -1 ) ;
        motor::vector< T > _buffer = motor::vector< T >( num_entries ) ;

    public:

        ring_buffer( void_t ) noexcept {}

        ring_buffer( ring_buffer const & rhv ) noexcept
        {
            _buffer = rhv._buffer ;
            _cur_pos = rhv._cur_pos ;
        }

        ring_buffer( ring_buffer && rhv ) noexcept
        {
            _buffer = std::move( rhv._buffer ) ;
            _cur_pos = rhv._cur_pos ;
            rhv._cur_pos = size_t( -1 ) ;
        }

        ring_buffer & operator = ( ring_buffer const & rhv ) noexcept
        {
            _buffer = rhv._buffer ;
            _cur_pos = rhv._cur_pos ;
            return *this ;
        }

        ring_buffer & operator = ( ring_buffer && rhv ) noexcept
        {
            _buffer = std::move( rhv._buffer ) ;
            _cur_pos = rhv._cur_pos ;
            rhv._cur_pos = size_t( -1 ) ;
            return *this ;
        }

    public:

        T & operator [] ( size_t const i ) noexcept
        {
            return _buffer[ i % num_entries ] ;
        }

        T const & operator [] ( size_t const i ) const noexcept
        {
            return _buffer[ i % num_entries ] ;
        }

        void_t append( T const & v ) noexcept
        {
            _cur_pos = ( ++_cur_pos ) % num_entries ;
            _buffer[ _cur_pos ] = v ;
        }

        void_t append( T && v ) noexcept
        {
            _cur_pos = ( ++_cur_pos ) % num_entries ;
            _buffer[ _cur_pos ] = std::move( v ) ;
        }

        size_t size( void_t ) const noexcept { return num_entries ; }

        size_t cur_pos( void_t ) const noexcept { return _cur_pos ; }
    };
} 
    template< typename T, size_t num_entries >
    using ring_buffer = mstd::ring_buffer< T, num_entries > ;
}