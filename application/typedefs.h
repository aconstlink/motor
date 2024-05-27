#pragma once

#include <motor/std/string>
#include <motor/std/vector>
#include <motor/memory/global.h>
#include <motor/core/types.hpp>
#include <motor/core/macros/typedef.h>

namespace motor
{
    namespace application
    {
        using namespace motor::core::types ;

        enum class network_client_type
        {
            udp,
            tcp
        };
    }
}