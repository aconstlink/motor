
// SPDX-FileCopyrightText: 2023-2026 Alex Link
// SPDX-License-Identifier: MIT

#pragma once

#include "aabb.hpp"
#include "obb.hpp"

namespace motor::collide::c3d
{
template < typename T >
struct transform
{
    using vec4_t = motor::math::vector4< T > ;

    // this function can be used to transform an aabb to form a obb. 
    // if you want the come back to an aabb use the next function bellow transform_bounds::to_aabb
    static motor::collide::c3d::obb< T > from_aabb( motor::collide::c3d::aabb< T > const & b,
        motor::math::m3d::transformation< T > const & trafo ) noexcept
    {
        auto const o = (trafo * vec4_t( b.get_center(), T(1) )).xyz() ;
        auto const e = b.get_extend();

        return motor::collide::c3d::obb< T >( o, e, trafo.get_rotation_matrix() );
    }
};

template < typename T >
struct transform_bounds
{
    using vec3_t = motor::math::vector3< T >;
    using aabb_t = motor::collide::c3d::aabb< T >;
    using obb_t = motor::collide::c3d::obb< T >;

    // this function function transforms an obb back to an aabb.
    // the resulting aabb might be a little bit bigger than the enclosing
    // object, but it is very fast!
    static aabb_t to_aabb( obb_t const & box ) noexcept
    {
        auto const o = box.origin();
        auto const a = box.x().absed() + box.y().absed() + box.z().absed();        

        vec3_t const max = o + a ;
        vec3_t const min = o - a ;

        return aabb_t( max, min );
    }
};

} // namespace motor::collide::c3d

namespace motor::collide
{
    motor_typedefs( c3d::transform< float_t >, transform3f ) ;
    motor_typedefs( c3d::transform_bounds< float_t >, transform_bounds3f ) ;
}