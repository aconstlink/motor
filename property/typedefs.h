#pragma once

#include <motor/std/vector>
#include <motor/base/types.hpp>
#include <motor/base/macros/typedef.h>
#include <motor/base/macros/move.h>

#include <motor/math/vector/vector2.hpp>
#include <motor/math/vector/vector3.hpp>
#include <motor/math/vector/vector4.hpp>

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