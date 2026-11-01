// SPDX-FileCopyrightText: 2023-2026 Alex Link
// SPDX-License-Identifier: MIT

#pragma once

#include "../../hit_test_types.h"
#include "../bounds/aabb.hpp"

namespace motor
{

namespace collide
{
namespace c3d
{
/// @returns
/// ht_outside
/// ht_intersect
template < typename type_t >
static hit_test_type hit_test_box_box( aabb< type_t > const & a, aabb< type_t > const & b )
{
    if( ( a.get_min().greater_than( b.get_max() ) ).any() ) return motor::collide::ht_outside;
    if( ( b.get_min().greater_than( a.get_max() ) ).any() ) return motor::collide::ht_outside;

    return ht_intersect;
}

///
/// algorithm from rtr p. 764
template < typename type_t >
static bool_t intersect( aabb< type_t > const & volume_a, aabb< type_t > const & volume_b )
{
    return hit_test_box_box( volume_a, volume_b ) == ht_intersect;
}
} // namespace c3d
} // namespace collide

} // namespace motor
