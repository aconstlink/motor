

#pragma once

#include "../utility/fn.hpp"
#include <functional>

namespace motor
{
    namespace math
    {
        enum class time_remap_funk_type
        {
            none,
            clamp,
            cycle,
            num_time_funk_types
        };

        // compute new time value
        // time_funk_t( t, begin, end ) -> new_t
        using time_remap_funk_t = std::function< size_t ( size_t const t, size_t const a, size_t const b ) > ;

        namespace detail
        {
            static const time_remap_funk_t time_remap_funks[] =
            {
                // none
                []( size_t const t, size_t const /*a*/, size_t const /*b*/ )
                {
                    return t ;
                },
                    // clamp
                    []( size_t const t, size_t const a, size_t const b )
                {
                    return motor::math::fn< size_t >::clamp( t, a, b ) ;
                },
                    // cycle
                    []( size_t const t, size_t const a, size_t const b )
                {
                    return ( t - a ) % ( b - a ) ;
                }
            } ;
        }

        static time_remap_funk_t get_time_remap_funk( motor::math::time_remap_funk_type const trf ) noexcept
        {
            return detail::time_remap_funks[ size_t( trf ) % size_t( time_remap_funk_type::num_time_funk_types ) ] ;
        }
    }
}