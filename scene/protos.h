#pragma once

#include <motor/base/macros/typedef.h>

namespace motor
{
    namespace scene
    {
        motor_class_proto_typedefs( ivisitable ) ;
        


        motor_class_proto_typedefs( node ) ;
        motor_class_proto_typedefs( group ) ;
        motor_class_proto_typedefs( leaf ) ;
        motor_class_proto_typedefs( decorator ) ;
        motor_class_proto_typedefs( logic_group ) ;
        motor_class_proto_typedefs( logic_decorator ) ;
        motor_class_proto_typedefs( trafo3d_node ) ;
        motor_class_proto_typedefs( camera_node ) ;

        motor_class_proto_typedefs( ivisitor ) ;
        motor_class_proto_typedefs( icomponent ) ;
    }
}
