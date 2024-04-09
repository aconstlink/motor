

#pragma once

#include <motor/std/vector>

namespace motor { namespace mstd {

    template< typename T >
    class histogram
    {
        using this_t = histogram< T > ;
        using this_rref_t = this_t && ;
        using this_cref_t = this_t const & ;

        using value_t = T ;

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

        histogram( void ) noexcept {}

        void insert( value_t const v ) noexcept
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

        void remove( value_t const v ) noexcept
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

        using for_each_funk_t = std::function< void ( size_t const i, this_t::data_cref_t ) > ;
        void for_each_entry( for_each_funk_t funk ) const noexcept
        {
            size_t i = 0; 
            for ( auto const & d : _histo )
            {
                funk( i++, d ) ;
            }
        }

        size_t get_num_entries( void ) const noexcept { return _histo.size() ; }
        size_t get_max_count( void ) const noexcept { return _max_count ;  }

        void clear( void ) noexcept
        {
            _histo.clear() ;
            _max_count = 0 ;
        }
    };
} }