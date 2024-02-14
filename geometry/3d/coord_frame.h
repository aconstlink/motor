
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
        class MOTOR_GEOMETRY_API coord_frame
        {
        public:

            struct input_params
            {
                motor::math::vec3f_t scale = motor::math::vec3f_t(1.0f) ;
            };
            motor_typedef( input_params ) ;

        public:

            static motor::geometry::result make( flat_line_mesh_ptr_t, input_params_cref_t ) ;
        };
        motor_typedef( coord_frame ) ;
    }
}