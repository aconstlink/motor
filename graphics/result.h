
#pragma once

#include <motor/base/macros/typedef.h>

namespace motor
{
    namespace graphics
    {
        enum class result
        {
            ok, 
            failed,
            invalid,
            invalid_argument,
            in_transit
        };
        motor_typedef( result ) ;


        enum class async_result
        {
            ok,
            failed,
            invalid,
            in_transit,
            user_edit
        };
    }
}