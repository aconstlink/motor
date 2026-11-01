// SPDX-FileCopyrightText: 2023-2026 Alex Link
// SPDX-License-Identifier: MIT

#pragma once

#include "../../hit_test_types.h"
#include "../bounds/aabb.hpp"

namespace motor::collide::c2d
{
template < typename T >
struct aabb_aabb_hit_test
{
    motor_typedefs( motor::collide::c2d::aabb< T >, aabb );

    /// @returns
    /// ht_outside
    /// ht_intersect
    static hit_test_type hit_test_box_box( aabb_cref_t a, aabb_cref_t b )
    {
        if( ( a.get_min().greater_than( b.get_max() ) ).any() )
            return motor::collide::hit_test_type::outside;

        if( ( b.get_min().greater_than( a.get_max() ) ).any() )
            return motor::collide::hit_test_type::outside;

        return motor::collide::hit_test_type::intersect;
    }

    static bool_t intersect( aabb_cref_t a, aabb_cref_t b )
    {
        return hit_test_box_box( a, b ) == motor::collide::hit_test_type::intersect;
    }
};
} // namespace motor::collide::c2d
