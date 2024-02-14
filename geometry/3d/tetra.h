
#pragma once

#include "../result.h"
#include "../typedefs.h"
#include "../protos.h"
#include "../api.h"

#include <motor/math/vector/vector2.hpp>
#include <motor/math/vector/vector3.hpp>

namespace motor
{
    namespace geometry
    {
        struct MOTOR_GEOMETRY_API tetra
        {
            struct input_params
            {
                /// per dimension scale in object space!
                motor::math::vec3f_t scale = motor::math::vec3f_t( 1.0f );
                /// per dimension rotation - euler angles in object space!
                motor::math::vec3f_t euler = motor::math::vec3f_t( 0.0f );
            };
            motor_typedef( input_params ) ;

            static motor::geometry::result make( polygon_mesh_ptr_t, input_params_cref_t ) ;
            
        };
    }
}
