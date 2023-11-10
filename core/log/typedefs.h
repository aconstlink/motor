#pragma once

#include <motor/core/types.hpp>
#include <motor/core/macros/common.h>
#include <motor/core/macros/typedef.h>
#include <motor/core/macros/move.h>
#include <motor/core/std/string>

namespace motor
{
    namespace log
    {
        using namespace motor::core::types ;

        motor_class_proto_typedefs( system );
        motor_class_proto_typedefs( ilogger );
        motor_class_proto_typedefs( std_cerr_logger );
        motor_class_proto_typedefs( store_logger );
        motor_class_proto_typedefs( system );
    }
}