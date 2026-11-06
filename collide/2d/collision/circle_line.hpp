// SPDX-FileCopyrightText: 2023-2026 Alex Link
// SPDX-License-Identifier: MIT

#pragma once

#include "../../typedefs.h"

#include "../bounds/circle.hpp"

#include <motor/math/vector/vector2.hpp>

#include <motor/math/primitive/2d/ray_line_intersect.hpp>

namespace motor::collide::c2d
{
template < typename type_t >
class circle_line_collision
{
  public:

    typedef motor::collide::c2d::circle< type_t > circle_t;
    typedef circle_t const & circle_cref_t;

    typedef motor::math::c2d::ray< type_t > ray_t;
    typedef ray_t const & ray_cref_t;

    typedef motor::math::c2d::line< type_t > line_t;
    typedef line_t const & line_cref_t;

    typedef motor::math::vector2< type_t > vec2_t;
    typedef vec2_t const & vec2_cref_t;

  public:

    /// @note
    /// if t_on_ray < 0, the result is invalid.
    struct collision_result
    {
        collision_result( vec2_cref_t corrected, vec2_cref_t poc, type_t t_ )
            : corrected_position( corrected ), point_of_collision( poc ), t_on_ray( t_ )
        {
        }

        /// the corrected circle in case of penetration.
        vec2_t corrected_position;

        /// point of collision of circle with the line.
        vec2_t point_of_collision;

        /// t times the ray of movement where the
        /// circle hits the line.
        /// if < 0, invalid result
        type_t t_on_ray;
    };

  public:

    /// collide a circle c on the ray of movement r on the line l.
    static collision_result collide( circle_cref_t c, ray_cref_t r, line_cref_t l )
    {
        const type_t at = c.get_radius();
        const vec2_t n = l.get_normal();

        if( n.dot( r.get_direction() ) > type_t( 0 ) )
            return collision_result( vec2f_t(), vec2f_t(), type_t( -1 ) );

        const type_t t_at_d =
            motor::math::c2d::ray_line_intersection< type_t >::intersect( at, r, l );

        /// is the point on distance d from line l.
        const vec2_t pod = r.point_at( t_at_d );

        /// is the point on the line in direction -n from pod.
        /// i.e. the point of collision.
        const vec2_t poc = pod - n * at;

        return collision_result( pod, poc, t_at_d );
    }
};
} // namespace motor::collide::c2d
