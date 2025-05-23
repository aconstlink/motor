#pragma once

#include <motor/base/types.hpp>
#include <motor/base/macros/typedef.h>
#include <motor/base/macros/move.h>

#include <string>

namespace motor
{
    namespace format
    {
        using namespace motor::core ;
        using namespace motor::core::types ;

        motor_class_proto_typedefs( module_registry ) ;
    }
}