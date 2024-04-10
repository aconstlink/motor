#pragma once

#include <motor/core/types.hpp>
#include <motor/core/macros/typedef.h>
#include <motor/core/macros/move.h>

#include <chrono>

namespace motor
{
    namespace profiling
    {
        using namespace motor::core ;
        using namespace motor::core::types ;

        using clock_t = std::chrono::high_resolution_clock ;
    }
}