
#pragma once

#include "api.h"
#include "typedefs.h"

#include <motor/math/vector/vector2.hpp>

namespace motor
{
    namespace physics
    {
        struct particle
        {
            float_t age = 1.0f ; // in seconds
            
            float_t mass = 1.0f ;

            motor::math::vec2f_t force ;

            motor::math::vec2f_t pos ;
            motor::math::vec2f_t vel ;
            motor::math::vec2f_t acl ;
        };
        motor_typedef( particle ) ;
    }
}
