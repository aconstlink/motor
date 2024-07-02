

#pragma once

#include <motor/std/vector>

#include <motor/core/types.hpp>
#include <motor/core/macros/typedef.h>
#include <motor/core/macros/move.h>

namespace motor
{
    namespace wire
    {
        using namespace motor::core ;
        using namespace motor::core::types ;

        
        motor_class_proto_typedefs( isignal ) ;
        motor_class_proto_typedefs( islot ) ;

        template< typename T >
        class signal ;

        template< typename T >
        class slot ;
    }
}