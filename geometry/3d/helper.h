
#pragma once

#include "../typedefs.h"

#include <motor/math/vector/vector3.hpp>

namespace motor
{
    namespace geometry
    {
        struct helper_3d
        {        
            /// @param index the vector index
            static void_t vector_to_array( size_t index, motor::math::vec3f_cref_t in_vec, floats_ref_t to ) 
            {
                to[index+0] = in_vec.x() ;
                to[index+1] = in_vec.y() ;
                to[index+2] = in_vec.z() ;
            }
        };
    }
}
