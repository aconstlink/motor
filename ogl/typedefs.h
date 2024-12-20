#pragma once

#include <motor/memory/global.h>
#include <motor/memory/allocator.hpp>

#include <motor/base/types.hpp>
#include <motor/base/macros/typedef.h>

#include <algorithm>

namespace motor
{
    namespace ogl
    {
        using namespace motor::core::types ;
        using memory = motor::memory::global ;

        template< typename T >
        using allocator = motor::memory::allocator< T, motor::ogl::memory > ;
    }
}