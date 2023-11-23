#pragma once

#include "../typedefs.h"

namespace motor
{
    namespace math
    {
        template< typename T >
        struct constants
        {
            motor_this_typedefs( constants<T> ) ;
            motor_typedefs( T, type ) ;

            static type_t pi( void_t )
            {
                return type_t( 3.141592653589793238462643383279502884197169399375105 ) ;
            }

            static type_t pix2( void_t )
            {
                return ( type_t( 2.0 ) * this_t::pi() ) ;
            }
        };
    }
}
