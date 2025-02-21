#pragma once

#include <motor/memory/global.h>
#include <motor/memory/allocator.hpp>
#include <motor/std/vector_pod>

#include <motor/base/types.hpp>
#include <motor/base/macros/typedef.h>
#include <motor/base/macros/move.h>
#include <motor/memory/typedefs.h>

#define MOTOR_PHYSICS_USE_PARALLEL_FOR 1 

namespace motor
{
    namespace physics
    {
        using namespace motor::core ;
        using namespace motor::core::types ;

        enum class spawn_location_type
        {
            area,
            border
        } ;

        enum class spawn_distribution_type
        {
            uniform,
            random
        };

        enum class variation_type
        {
            fixed,
            random
        };
    }
}