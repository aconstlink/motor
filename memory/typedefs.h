#pragma once

#include <motor/core/types.hpp>
#include <motor/core/macros/common.h>
#include <motor/core/macros/typedef.h>
#include <motor/core/macros/move.h>

#include <functional>

namespace motor
{
    using namespace motor::core::types ;

    namespace memory
    {
        using namespace motor::core::types ;
        using void_funk_t = std::function< void_t ( void_t ) > ;
    }
}