#pragma once

#include "factorial.hpp"

namespace motor
{
    namespace math
    {
        template< typename T, size_t N >
        struct taylor_series
        {
            static T sin( T const x )
            {
                return T( std::pow( x, T( 2 * N + 1 ) ) ) * constexpr( T( std::pow( T( -1 ), T( N ) ) ) ) /
                    T( motor::math::factorial<constexpr( ( N << 1 ) + 1 )>::value ) +
                    taylor_series<T, N - 1>::sin( x ) ;
            }

            static T cos( T const x )
            {
                return T( std::pow( x, T( N << 1 ) ) ) * constexpr( T( std::pow( T( -1 ), T( N ) ) ) ) /
                    T( motor::math::factorial<constexpr( N << 1 )>::value ) +
                    taylor_series<T, N - 1>::cos( x ) ;
            }
        };

        template< typename T >
        struct taylor_series< T, 0 >
        {
            static T sin( T const x )
            {
                return x  ;
            }

            static T cos( T const x )
            {
                return 1  ;
            }
        };
    }
}