// SPDX-FileCopyrightText: 2023-2026 Alex Link
// SPDX-License-Identifier: MIT


#pragma once

#include <motor/base/macros/typedef.h>

namespace motor
{
    namespace geometry
    {
        motor_class_proto_typedefs( imesh ) ;
        motor_struct_proto_typedefs( tri_mesh ) ;
        motor_class_proto_typedefs( polygon_mesh ) ;
        motor_struct_proto_typedefs( flat_tri_mesh ) ;

        motor_class_proto_typedefs( iwire ) ;
        motor_class_proto_typedefs( flat_line_mesh ) ;
    }
}
