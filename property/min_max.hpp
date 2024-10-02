

#pragma once

#include "typedefs.h"

namespace motor
{
    namespace property
    {
        // @param T the value type
        // @param enable true if this class should be enabled
        //          otherwise it is empty
        template< typename T, bool enable >
        class min_max {};

        // disabled => enable = false
        template< typename T >
        class min_max < T, false > {};

        template< typename T >
        class min_max< T, true >
        {
        private:

            T _min ;
            T _max ;

        public:

            min_max( void_t ) noexcept {}

            min_max( std::pair< T, T > const & p ) noexcept 
            {
                _min = p.first ;
                _max = p.second ;
            }

            min_max( T min_, T max_ ) noexcept : _min( min_ ), _max( max_ ) 
            {
            }

            ~min_max( void_t ) noexcept {}

            T get_min( void_t ) const noexcept { return _min ; }
            T get_max( void_t ) const noexcept { return _max ; }

            void_t set_min( T const & v ) noexcept { _min = v ; }
            void_t set_max( T const & v ) noexcept { _max = v ; }
        };
    }
}