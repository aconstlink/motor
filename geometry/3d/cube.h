
#pragma once

#include "../result.h"
#include "../typedefs.h"
#include "../protos.h"
#include "../api.h"

#include <motor/math/vector/vector3.hpp>

namespace motor
{
    namespace geometry
    {
        struct MOTOR_GEOMETRY_API cube
        {
            struct input_params
            {
                /// per dimension scale in object space!
                motor::math::vec3f_t scale = motor::math::vec3f_t(1.0f);
                /// per dimension rotation - euler angles in object space!
                motor::math::vec3f_t euler = motor::math::vec3f_t(0.0f);

                /// tessellation :
                /// vertices per side (vps) = tess + 1
                /// vertices per face (vpf) = vps * vps 
                /// vertices per cube (vpc) = vpf * 6 
                size_t tess = 1 ;
            };
            motor_typedef( input_params ) ;

            static motor::geometry::result make( polygon_mesh_ptr_t, input_params_cref_t ) ;
            static motor::geometry::result make( tri_mesh_ptr_t, input_params_cref_t ) ;
            static motor::geometry::result make( flat_line_mesh_ptr_t, input_params_cref_t ) ;
        };
        motor_typedef( cube ) ;
    }
}