

#pragma once

#include <motor/std/vector>

#include <motor/base/types.hpp>
#include <motor/base/macros/typedef.h>
#include <motor/base/macros/move.h>

namespace motor
{
    namespace wire
    {
        using namespace motor::core ;
        using namespace motor::core::types ;

        
        motor_class_proto_typedefs( inode ) ;
        motor_class_proto_typedefs( ioutput_slot ) ;
        motor_class_proto_typedefs( iinput_slot ) ;

        template< typename T >
        class output_slot ;

        template< typename T >
        class input_slot ;
    }
}