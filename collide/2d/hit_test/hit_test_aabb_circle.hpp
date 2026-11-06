// SPDX-FileCopyrightText: 2023-2026 Alex Link
// SPDX-License-Identifier: MIT

#pragma once

#include "../../typedefs.h"
#include "../../hit_test_types.h"

#include "../bounds/aabb.hpp"
#include "../bounds/circle.hpp"

#include <motor/math/vector/vector4.hpp>
#include <motor/math/vector/vector4b.hpp>

namespace motor::collide::c2d
{
/// @return true, if circle and aabb overlap, otherwise false.
template < typename type_t >
static bool_t hit_test_box_circle_overlap(
    aabb< type_t > const & bound_aabb, circle< type_t > const & bound_circle )
{
    const type_t d = bound_aabb.squared_distance_to( bound_circle.get_center() );
    const type_t r = bound_circle.get_radius2();

    return d <= r;
}

/// @return true, if circle and aabb overlap, otherwise false.
/// @return closest point to circle on intersection. Invalid if function returns false.
template < typename type_t >
static bool_t hit_test_box_circle_overlap( aabb< type_t > const & bound_aabb,
    circle< type_t > const & bound_circle, motor::math::vector2< type_t > & closest_point )
{
    bound_aabb.closest_point_to( bound_circle.get_center(), closest_point );
    return ( closest_point - bound_circle.get_center() ).length2() <= bound_circle.get_radius2();
}

/// box against sphere
/// this function also returns intersect, if the sphere is completely inside the box.
/// @return
/// ht_outside: if box and sphere are completely outside of each other
/// ht_inside: the box is completely inside of the sphere
/// ht_intersect: both box' and sphere's surface intersect
template < typename type_t >
static hit_test_type hit_test_box_circle(
    c2d::aabb< type_t > const & volume_a, c2d::circle< type_t > const & volume_b )
{
    typedef motor::math::vector2< type_t > vec2_t;

    const vec2_t max = volume_b.get_center() - volume_a.get_max();
    const vec2_t min = volume_b.get_center() - volume_a.get_min();
    const type_t radius = volume_b.get_radius();

    vec2_t inner;
    vec2_t outer;

    if( min.x() < type_t( 0 ) )
    {
        if( min.x() < -radius ) return ht_outside;
        inner.x() = min.x();
    }
    else if( max.x() > type_t( 0 ) )
    {
        if( max.x() > radius ) return ht_outside;
        inner.x() = max.x();
    }
    if( min.y() < type_t( 0 ) )
    {
        if( min.y() < -radius ) return ht_outside;
        inner.y() = min.y();
    }
    else if( max.y() > type_t( 0 ) )
    {
        if( max.y() > radius ) return ht_outside;
        inner.y() = max.y();
    }

    if( inner.length2() > radius * radius ) return ht_outside;

    // construct the outer vector
    inner = volume_b.get_center() - inner;
    outer = volume_a.get_center() - inner;
    outer = inner + outer * type_t( 2 );

    // if outer vector < radius^2, the box is inside the sphere.
    if( ( outer - volume_b.get_center() ).length2() < radius * radius ) return ht_inside;

    return ht_intersect;
}

} // namespace motor::collide::c2d
