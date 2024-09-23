#pragma once

#include <motor/base/types.hpp>
#include <motor/base/macros/common.h>
#include <motor/base/macros/typedef.h>
#include <motor/base/macros/move.h>

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