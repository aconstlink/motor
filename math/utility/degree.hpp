#pragma once

#include "angle.hpp"
#include "constants.hpp"

namespace motor
{    
    namespace math
    {
        template< typename T >
        class degree
        {
            motor_this_typedefs( degree<T> ) ;
            motor_typedefs( T, type ) ;

            motor_typedefs( angle<T>, angle ) ;

        public:

            static type_t val_to_radian( type_t const degree )
            {
                return degree * motor::math::constants<type_t>::pi() / type_t( 180 ) ;
            }

            static angle_t to_radian( type_t const deg )
            {
                return angle_t( this_t::val_to_radian( deg ) ) ;
            }

        };
    }
}