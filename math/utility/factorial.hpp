#pragma once

namespace motor
{
    namespace math
    {
        template< size_t N >
        struct factorial
        {
            static const size_t value = N * factorial< N - 1 >::value ;
        };

        template<>
        struct factorial<0>
        {
            static const size_t value = 1 ;
        };
    }
}