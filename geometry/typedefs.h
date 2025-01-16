
#pragma once

#include <motor/memory/global.h>
#include <motor/memory/allocator.hpp>

#include <motor/std/vector>
#include <motor/std/vector_pod>
#include <motor/std/string>

#include <motor/math/vector/vector2.hpp>
#include <motor/math/vector/vector3.hpp>
#include <motor/math/vector/vector4.hpp>

#include <motor/base/types.hpp>
#include <motor/base/macros/typedef.h>

namespace motor
{
    namespace geometry
    {
        using namespace motor::core::types ;

        motor_typedefs( motor::string_t, string ) ;
        motor_typedefs( motor::string_t, key ) ;

        motor_typedefs( motor::vector_pod< uint_t >, uints ) ;
        motor_typedefs( motor::vector< uints_t >, more_uints ) ;

        motor_typedefs( motor::vector_pod< float_t >, floats ) ;
        motor_typedefs( motor::vector< floats_t >, more_floats ) ;
    }
     
}