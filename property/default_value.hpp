

#pragma once

#include <motor/std/string>

namespace motor
{
    namespace property
    {
        template< typename T >
        static T default_value( void_t ) noexcept
        {
            return T(0) ;
        };

        template<>
        motor::string_t default_value< motor::string_t >( void_t ) noexcept
        {
            return "" ;
        };
    }
}