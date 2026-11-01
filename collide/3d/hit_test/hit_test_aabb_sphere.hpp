// SPDX-FileCopyrightText: 2023-2026 Alex Link
// SPDX-License-Identifier: MIT
#pragma once

#include "../../hit_test_types.h"

#include "../bounds/aabb.hpp"
#include "../bounds/sphere.hpp"

namespace motor
{
namespace collide
{
namespace c3d
{
using c3d::aabb;
using c3d::sphere;

/// @returns
/// ht_disjoint
/// ht_overlap
template < typename type_t >
static hit_test_type hit_test_box_sphere_overlap(
    aabb< type_t > const & volume_a, sphere< type_t > const & volume_b )
{
    typedef motor::math::vector3< type_t > vec3_t;

    const vec3_t max = volume_b.get_center() - volume_a.get_max();
    const vec3_t min = volume_b.get_center() - volume_a.get_min();
    const type_t radius = volume_b.get_radius();

    type_t d = type_t( 0 );

    if( min.x() < type_t( 0 ) )
    {
        if( min.x() < -radius ) return ht_disjoint;
        d += min.x() * min.x();
    }
    else if( max.x() > type_t( 0 ) )
    {
        if( max.x() > radius ) return ht_disjoint;
        d += max.x() * max.x();
    }
    if( min.y() < type_t( 0 ) )
    {
        if( min.y() < -radius ) return ht_disjoint;
        d += min.y() * min.y();
    }
    else if( max.y() > type_t( 0 ) )
    {
        if( max.y() > radius ) return ht_disjoint;
        d += max.y() * max.y();
    }
    if( min.z() < type_t( 0 ) )
    {
        if( min.z() < -radius ) return ht_disjoint;
        d += min.z() * min.z();
    }
    else if( max.z() > type_t( 0 ) )
    {
        if( max.z() > radius ) return ht_disjoint;
        d += max.z() * max.z();
    }

    if( d > radius * radius ) return ht_disjoint;

    return ht_overlap;
}

/// box against sphere
/// this function also returns intersect, if the sphere is completely inside the box.
/// @return
/// ht_outside: if box and sphere are completely outside of each other
/// ht_inside: the box is completely inside of the sphere
/// ht_intersect: both box' and sphere's surface intersect
template < typename type_t >
static hit_test_type hit_test_box_sphere(
    aabb< type_t > const & volume_a, sphere< type_t > const & volume_b )
{
    typedef motor::math::vector3< type_t > vec3_t;

    const vec3_t max = volume_b.get_center() - volume_a.get_max();
    const vec3_t min = volume_b.get_center() - volume_a.get_min();
    const type_t radius = volume_b.get_radius();

    vec3_t inner;
    vec3_t outer;

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
    if( min.z() < type_t( 0 ) )
    {
        if( min.z() < -radius ) return ht_outside;
        inner.z() = min.z();
    }
    else if( max.z() > type_t( 0 ) )
    {
        if( max.z() > radius ) return ht_outside;
        inner.z() = max.z();
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

/// is volume a intersecting with volume b
/// algorithm from rtr p. 764
/// but modified so that at the end, the outer vector
/// is calculated in order to test intersection with surface only
template < typename type_t >
static bool_t intersect( aabb< type_t > const & volume_a, sphere< type_t > const & volume_b )
{
    return hit_test_box_sphere( volume_a, volume_b ) == ht_intersect;
}

/// is volume a inside volume b
/*template< typename type_t >
static bool_t inside( aabb<type_t> const & volume_a, sphere<vec_t> const & volume_b )
{
    hit_test_type t = hit_test_box_sphere(volume_a,volume_b) ;
    return  t == ht_intersect || t == ht_inside ;
}*/

} // namespace c3d
} // namespace collide

} // namespace motor