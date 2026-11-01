// SPDX-FileCopyrightText: 2023-2026 Alex Link
// SPDX-License-Identifier: MIT

#pragma once

#include "../../hit_test_types.h"

#include "../bounds/aabb.hpp"

#include <motor/math/planar/plane/hesse_plane3.hpp>

namespace motor 
{
    namespace collide
    {
    namespace c3d
    {
        /// test if a point is inside the box.
        /// @return
        /// ht_outside: point is not in the box
        /// ht_inside: point is in the box
        template< typename type_t >
        static hit_test_type hit_test_box_point( aabb< type_t > const & box, motor::math::vector3< type_t > const & point )
        {
            typedef motor::math::vector3< type_t > vec3_t ;

			vec3_t point_a = (point-box.get_center()).absed() ;
			return point_a < box.get_extend() ? ht_inside : ht_outside ;
        }

    }
    }
    
}

