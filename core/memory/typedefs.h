#pragma once

#include <motor/core/types.hpp>
#include <motor/core/macros/common.h>
#include <motor/core/macros/typedef.h>
#include <motor/core/macros/move.h>
#include <motor/core/std/string>

namespace motor
{
    namespace memory
    {
        using namespace motor::core::types ;

        motor_typedefs( motor::core::string_t, purpose ) ;
        motor_class_proto_typedefs( imanager ) ;
    }
}