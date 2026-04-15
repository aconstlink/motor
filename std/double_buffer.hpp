

#pragma once

#include "vector"
#include <motor/base/types.hpp>

namespace motor { namespace mstd {

    using namespace motor::core::types ;

    template< typename T >
    class double_buffer
    {
        motor_this_typedefs( double_buffer< T > ) ;

        using buffer_t = motor::vector< T > ;

    private:

        size_t _idx = 0 ;
        buffer_t _buffers[2] ;


        size_t read_idx( void_t ) const noexcept
        {
            return _idx ;
        }

        size_t write_idx( void_t ) const noexcept
        {
            return (_idx + 1) % 2 ;
        }


        

    public:


        double_buffer( void_t ) noexcept {}

        double_buffer( this_cref_t rhv ) noexcept : _buffers{ rhv._buffers[0],
            rhv._buffers[1] }, _idx( rhv._idx ) {}

        //double_buffer( this_rref_t rhv ) noexcept : _buffers[0]( std::move( rhv._buffers[0] ) ),
          //  _buffers[1]( std::move( rhv._buffers[1] ) ), _idx( rhv._idx ) {}
        
        double_buffer( this_rref_t rhv ) noexcept : _buffers{ std::move( rhv._buffers[0] ),
            std::move( rhv._buffers[1] ) }, _idx( rhv._idx ) {}


        void_t swap_and_clear( void_t ) noexcept
        {
            _idx = (_idx + 1) % 2 ;
            this_t::write_buffer().clear() ;
        }

        buffer_t & read_buffer( void_t ) noexcept { return _buffers[this_t::read_idx()] ; }
        buffer_t const & read_buffer( void_t ) const noexcept { return _buffers[this_t::read_idx()] ; }

        buffer_t & write_buffer( void_t ) noexcept { return _buffers[this_t::write_idx()] ; }
        buffer_t const & write_buffer( void_t ) const noexcept { return _buffers[this_t::write_idx()] ; }


    public:

        void_t write( T const & d ) noexcept { this_t::write_buffer().emplace_back( d ) ; }
        void_t write( T && d ) noexcept { this_t::write_buffer().emplace_back( std::move( d ) ) ; }

    public:



    };

} } 
