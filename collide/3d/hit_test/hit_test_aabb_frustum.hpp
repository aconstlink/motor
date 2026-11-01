// SPDX-FileCopyrightText: 2023-2026 Alex Link
// SPDX-License-Identifier: MIT

#pragma once

#include "hit_test_aabb_plane.hpp"

#include "../bounds/frustum.hpp"

namespace motor
{
namespace collide
{
using namespace motor::math;

namespace c3d
{

/// hit tests box and view frustum
/// will test all planes in the view frustum
/// @return
/// ht_outside: the box is not in the frustum
/// ht_inside: the box is in the frustum
/// ht_intersect: the box and the frustum intersect
template < typename type_t >
static hit_test_type hit_test_box_frustum(
    c3d::aabb< type_t > const & box, c3d::frustum< type_t > const & frust )
{
    bool_t bintersect = false;
    hit_test_type ret = ht_outside;

    for( size_t i = 0; i < 6; ++i )
    {
        ret = hit_test_box_plane< type_t >( box, frust.get_plane( i ) );
        if( ret == ht_intersect ) bintersect = true;
        if( ret == ht_outside ) break;
    }

    if( ( ret != ht_outside ) && bintersect )
    {
        ret = ht_intersect;
    }
    return ret;
}
} // namespace c3d
} // namespace collide

} // namespace motor
