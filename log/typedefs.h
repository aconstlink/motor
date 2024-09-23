#pragma once

#include <motor/std/string>

#include <motor/base/types.hpp>
#include <motor/base/macros/common.h>
#include <motor/base/macros/typedef.h>
#include <motor/base/macros/move.h>

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