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
using c3d::aabb;

/// hit tests box and plane in hesse form
/// @return
/// ht_outside: the box is in the negative plane space. the box is behind of the plane
/// ht_inside: the box is in the positive plane space. the box is in front of the plane
/// ht_intersect: the box and the plane intersect
template < typename type_t >
static hit_test_type hit_test_box_plane(
    aabb< type_t > const & box, hesse_plane< type_t, 3 > const & hesse )
{
    typedef motor::math::vector3< type_t > vec3_t;

    const vec3_t center = ( box.get_max() + box.get_min() ) * type_t( 0.5 );
    const vec3_t half = ( box.get_max() - box.get_min() ) * type_t( 0.5 );
    const vec3_t abs_normal = hesse.get_abs_normal();
    type_t e = half.dot( abs_normal );
    type_t s = hesse.distance( center );
    if( s - e > type_t( 0 ) ) return ht_outside;
    if( s + e < type_t( 0 ) ) return ht_inside;
    return ht_intersect;
}

} // namespace c3d
} // namespace collide

} // namespace motor
