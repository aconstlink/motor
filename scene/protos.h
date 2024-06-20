#pragma once

#include <motor/core/macros/typedef.h>

namespace motor
{
    namespace scene
    {
        motor_class_proto_typedefs( node ) ;
        motor_class_proto_typedefs( group ) ;
        motor_class_proto_typedefs( decorator ) ;
        motor_class_proto_typedefs( leaf ) ;
        motor_class_proto_typedefs( logic_group ) ;
        motor_class_proto_typedefs( logic_decorator ) ;

        motor_class_proto_typedefs( transform_3d ) ;
        motor_class_proto_typedefs( renderable ) ;
        motor_class_proto_typedefs( render_state ) ;
        motor_class_proto_typedefs( camera ) ;

        motor_class_proto_typedefs( ivisitor ) ;
    }
}
