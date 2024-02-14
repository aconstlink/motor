
#pragma once

#include "../../typedefs.h"
#include "../../api.h"


#include <motor/std/vector>
#include <functional>

namespace motor
{
    namespace geometry
    {
        struct MOTOR_GEOMETRY_API fractal_3d
        {
            struct fractal_object
            {
                motor::math::vec3f_t pos ;
                float_t side_width ;
            };
            motor_typedef( fractal_object ) ;

            motor_this_typedefs( fractal_3d ) ;

            motor_typedefs( motor::vector<fractal_object>, objects ) ;

            typedef std::function< objects_t ( fractal_object_cref_t ) > divider_funk_t ;
            

            static objects_t menger_sponge( fractal_object_cref_t ) ;
            static objects_t sierpinski_tetra( fractal_object_cref_t ) ;

            static divider_funk_t menger_sponge_funk( void_t ) ;
            static divider_funk_t sierpinski_tetra_funk( void_t ) ;

            static objects_t iterate( divider_funk_t, fractal_object_cref_t, size_t ) ;
        };
    }
}
