

#pragma once

#include "api.h"
#include "typedefs.h"

#include <motor/std/vector>

namespace motor { namespace mstd {

    template< typename T >
    class histogram
    {
        motor_this_typedefs( histogram ) ;

        motor_typedefs( T, value ) ;

    public:

        struct data
        {
            value_t value ;
            size_t count ;
        };
        motor_typedef( data ) ;

        size_t _max_count = 0 ;

    private:

        motor::vector< data > _histo ;

    public:

        histogram( void_t ) noexcept {}

        void_t insert( value_t const v ) noexcept
        {
            auto iter = std::find_if( _histo.begin(), _histo.end(), [&] ( this_t::data_cref_t d )
            {
                return d.value >= v ;
            } ) ;

            if ( iter == _histo.end() || iter->value > v )
            {
                iter = _histo.insert( iter, this_t::data_t { v, 0 } ) ;
            }

            ++iter->count ;

            _max_count = std::max( _max_count, iter->count ) ;
        }

        void_t remove( value_t const v ) noexcept
        {
            auto iter = std::find_if( _histo.begin(), _histo.end(), [&] ( this_t::data_cref_t d )
            {
                return d.value == v ;
            } ) ;

            if ( iter == _histo.end() ) return ;

            assert( iter->count != 0 ) ;

            --iter->count ;

            _max_count = std::min( _max_count, iter->count ) ;

            if ( iter->count == 0 ) _histo.erase( iter ) ;
        }

        using for_each_funk_t = std::function< void_t ( size_t const i, this_t::data_cref_t ) > ;
        void_t for_each_entry( for_each_funk_t funk ) const noexcept
        {
            size_t i = 0; 
            for ( auto const & d : _histo )
            {
                funk( i++, d ) ;
            }
        }

        size_t get_num_entries( void_t ) const noexcept { return _histo.size() ; }
        size_t get_max_count( void_t ) const noexcept { return _max_count ;  }
    };
} }