

#pragma once

#include "aabb.hpp"
#include "sphere.hpp"

#include "../hit_test_types.h"

#include <motor/math/primitive/3d/ray.hpp>

#include <cmath>

namespace motor::collide::c3d
{
template < typename T >
struct hit_test
{
    motor_this_typedefs( hit_test< T > );

    using type_t = T;
    using aabb_t = motor::collide::c3d::aabb< T >;
    using sphere_t = motor::collide::c3d::sphere< T >;
    using vec3_t = motor::math::vector3< T >;

    static motor::collide::hit_test_type hit_test_box_box(
        this_t::aabb_t const & a, this_t::aabb_t const & b ) noexcept
    {
        if( ( a.get_min().greater_than( b.get_max() ) ).any() )
            return motor::collide::hit_test_type::outside;
        if( ( b.get_min().greater_than( a.get_max() ) ).any() )
            return motor::collide::hit_test_type::outside;

        return motor::collide::hit_test_type::intersect;
    }

    static bool_t intersect(
        this_t::aabb_t const & volume_a, this_t::aabb_t const & volume_b ) noexcept
    {
        return hit_test_box_box( volume_a, volume_b ) == intersect;
    }

    /// test if a point is inside the box.
    /// @return
    /// ht_outside: point is not in the box
    /// ht_inside: point is in the box
    static motor::collide::hit_test_type hit_test_box_point(
        this_t::aabb_t const & box, motor::math::vector3< type_t > const & point ) noexcept
    {
        auto const p = ( point - box.get_center() ).absed();
        auto const e = box.get_extend();
        auto const x = p.less_equal_than( e );

        return x.all() ? motor::collide::hit_test_type::inside
                       : motor::collide::hit_test_type::outside;
    }

    /// Points on the surface count as inside, matching the box/point test.
    static motor::collide::hit_test_type hit_test_sphere_point(
        this_t::sphere_t const & sphere, motor::math::vector3< type_t > const & point ) noexcept
    {
        auto const p = point - sphere.get_center();
        return p.length2() <= sphere.get_radius2() ? motor::collide::hit_test_type::inside
                                                  : motor::collide::hit_test_type::outside;
    }

    /// box/ray
    /// algorithm from rtr p. 743
    /// this test doesnt return the distance to the intersection point. there is a similar function
    /// though, which does that.
    /// @precondition
    /// the ray and the box need to be in the same space.
    /// the ray need to be normalized in direction.
    /// @return
    /// ht_outside: the ray does not intersect with the aabb box
    /// ht_intersect: the ray and the box intersect
    static hit_test_type hit_test_box_ray(
        aabb< type_t > const & box, motor::math::ray3f_t const & ray )
    {
        float t_min = std::numeric_limits< type_t >::min();
        float t_max = std::numeric_limits< type_t >::max();

        vec3_t p = box.get_center() - ray.get_origin();
        vec3_t h = box.get_max() - box.get_center();

        // for each axis
        for( size_t i = 0; i < 3; ++i )
        {
            type_t e = p[ i ];
            type_t f = ray.get_direction()[ i ];
            if( std::abs( f ) > std::numeric_limits< type_t >::epsilon() )
            {
                type_t t1 = ( e + h[ i ] ) / f;
                type_t t2 = ( e - h[ i ] ) / f;
                if( t1 > t2 )
                {
                    type_t tmp = t1;
                    t1 = t2;
                    t2 = tmp;
                }
                if( t1 > t_min ) t_min = t1;
                if( t2 < t_max ) t_max = t2;
                if( t_min > t_max ) return motor::collide::hit_test_type::outside;
                if( t_max < 0 ) return motor::collide::hit_test_type::outside;
            }
            else if( -e - h[ i ] > type_t( 0 ) || -e + h[ i ] < type_t( 0 ) )
                return motor::collide::hit_test_type::outside;
        }

        return motor::collide::hit_test_type::intersect;
    }

    /// box/ray
    /// algorithm from rtr p. 743
    /// this test intersects a box with a ray and returns the distance to the intersection point on
    /// the ray.
    /// @precondition
    /// the ray and the box need to be in the same space.
    /// the ray need to be normalized in direction.
    /// @return
    /// ht_outside: the ray does not intersect with the aabb box
    /// ht_intersect: the ray and the box intersect
    /// param t_out the distance to the intersection point on the ray if box and ray intersects.
    static hit_test_type hit_test_box_ray(
        this_t::aabb_t const & box, motor::math::ray3f_t const & ray, type_t & t_out ) noexcept
    {
        t_out = type_t( 0 );

        float t_min = std::numeric_limits< type_t >::min();
        float t_max = std::numeric_limits< type_t >::max();

        vec3_t const p = box.get_center() - ray.get_origin();
        vec3_t const h = box.get_max() - box.get_center();

        // for each axis
        for( size_t i = 0; i < 3; ++i )
        {
            type_t e = p[ i ];
            type_t f = ray.get_direction()[ i ];
            if( std::abs( f ) > std::numeric_limits< type_t >::epsilon() )
            {
                type_t t1 = ( e + h[ i ] ) / f;
                type_t t2 = ( e - h[ i ] ) / f;
                if( t1 > t2 )
                {
                    type_t tmp = t1;
                    t1 = t2;
                    t2 = tmp;
                }
                if( t1 > t_min ) t_min = t1;
                if( t2 < t_max ) t_max = t2;
                if( t_min > t_max ) return motor::collide::hit_test_type::outside;
                if( t_max < 0 ) return motor::collide::hit_test_type::outside;
            }
            else if( -e - h[ i ] > type_t( 0 ) || -e + h[ i ] < type_t( 0 ) )
                return motor::collide::hit_test_type::outside;
        }

        t_out = t_min > type_t( 0 ) ? type_t( t_min ) : type_t( t_max );
        return motor::collide::hit_test_type::intersect;
    }

    static bool_t intersect(
        aabb< type_t > const & box, motor::math::ray3f_t const & ray, type_t & t_out )
    {
        return this_t::hit_test_box_ray( box, ray ) == motor::collide::hit_test_type::intersect;
    }

    /// Returns the first surface hit with t >= 0, or the exit when starting inside.
    /// Both objects must be in the same space and the ray direction must be nonzero.
    /// t_out is the ray parameter (a distance for normalized directions), or zero on a miss.
    static hit_test_type hit_test_sphere_ray(
        this_t::sphere_t const & sphere, motor::math::ray3f_t const & ray, type_t & t_out ) noexcept
    {
        t_out = type_t( 0 );

        vec3_t const p = vec3_t( ray.get_origin() ) - sphere.get_center();
        vec3_t const d = ray.get_direction();
        type_t const a = d.dot( d );
        if( a == type_t( 0 ) ) return motor::collide::hit_test_type::outside;

        type_t const b = p.dot( d );
        type_t const c = p.dot( p ) - sphere.get_radius2();
        if( c > type_t( 0 ) && b > type_t( 0 ) )
            return motor::collide::hit_test_type::outside;

        type_t const discriminant = b * b - a * c;
        if( discriminant < type_t( 0 ) ) return motor::collide::hit_test_type::outside;

        type_t const root = std::sqrt( discriminant );
        type_t t = ( -b - root ) / a;
        if( t < type_t( 0 ) ) t = ( -b + root ) / a;
        if( t < type_t( 0 ) ) return motor::collide::hit_test_type::outside;

        t_out = t;
        return motor::collide::hit_test_type::intersect;
    }
};
} // namespace motor::collide::c3d
