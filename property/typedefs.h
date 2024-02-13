#pragma once

#include <motor/std/vector>
#include <motor/core/types.hpp>
#include <motor/core/macros/typedef.h>
#include <motor/core/macros/move.h>


namespace motor
{
    namespace property
    {
        using namespace motor::core ;
        using namespace motor::core::types ;


        motor_typedefs( uint32_t, utf32 ) ;
        motor_typedefs( motor::vector< utf32_t >, code_points ) ;
    }
}