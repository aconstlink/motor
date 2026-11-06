

#pragma once

#include "aabb.hpp"
#include "sphere.hpp"
#include "obb.hpp"

#include "../hit_test_types.h"

#include <motor/math/primitive/3d/ray.hpp>
#include <motor/math/primitive/3d/frustum.hpp>

#include <cmath>
#include <limits>

namespace motor::collide::c3d
{

// main book sources:
// Realtime Rendering
// Realtime Collision Detection
template < typename T >
struct hit_test
{
    motor_this_typedefs( hit_test< T > );

    using type_t = T;
    using aabb_t = motor::collide::c3d::aabb< T >;
    using sphere_t = motor::collide::c3d::sphere< T >;
    using obb_t = motor::collide::c3d::obb< T >;
    using frustum_t = motor::math::m3d::frustum< T >;
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

    /// The orientation must be orthonormal; scaling contains the half extents.
    /// Point and box must be in the same space. Surface points count as inside.
    static motor::collide::hit_test_type hit_test_obb_point(
        this_t::obb_t const & box, motor::math::vector3< type_t > const & point ) noexcept
    {
        vec3_t const p = point - box.origin();
        for( size_t i = 0; i < 3; ++i )
        {
            auto const axis = box.orientation().get_column( i );
            if( std::abs( p.dot( axis ) ) > std::abs( box.scaling()[ i ] ) )
                return motor::collide::hit_test_type::outside;
        }
        return motor::collide::hit_test_type::inside;
    }

    /// Frustum planes face outward; point and frustum must be in the same space.
    /// Points on the surface count as inside, matching the other point tests.
    static hit_test_type hit_test_frustum_point(
        this_t::frustum_t const & frustum, this_t::vec3_t const & point ) noexcept
    {
        for( size_t i = 0; i < size_t( frustum_t::frustum_plane::num_planes ); ++i )
        {
            auto const & plane = frustum.get_plane( i );
            if( plane.xyz().dot( point ) + plane.w() > type_t( 0 ) )
                return hit_test_type::outside;
        }
        return hit_test_type::inside;
    }

    /// Conservative six-plane culling with outward-facing unit-normal planes.
    /// Frustum and sphere must be in the same space; radius must be nonnegative.
    /// inside: sphere strictly inside; outside: separated by a frustum plane;
    /// intersect: touching or potentially overlapping (false positives near corners).
    static hit_test_type hit_test_frustum_sphere(
        this_t::frustum_t const & frustum, this_t::sphere_t const & sphere ) noexcept
    {
        auto result = hit_test_type::inside;
        type_t const radius = sphere.get_radius();
        for( size_t i = 0; i < size_t( frustum_t::frustum_plane::num_planes ); ++i )
        {
            auto const & plane = frustum.get_plane( i );
            type_t const distance = plane.xyz().dot( sphere.get_center() ) + plane.w();
            if( distance > radius ) return hit_test_type::outside;
            if( distance >= -radius ) result = hit_test_type::intersect;
        }
        return result;
    }

    /// Conservative six-plane culling with outward-facing planes, in the same space.
    /// inside: box strictly inside; outside: separated by a frustum plane;
    /// intersect: touching or potentially overlapping, not an exact overlap test.
    static hit_test_type hit_test_frustum_aabb(
        this_t::frustum_t const & frustum, this_t::aabb_t const & box ) noexcept
    {
        auto result = hit_test_type::inside;
        auto const center = box.get_center();
        auto const extent = box.get_extend();
        for( size_t i = 0; i < size_t( frustum_t::frustum_plane::num_planes ); ++i )
        {
            auto const & plane = frustum.get_plane( i );
            auto const normal = plane.xyz();
            type_t const distance = normal.dot( center ) + plane.w();
            type_t const radius = normal.absed().dot( extent );
            if( distance > radius ) return hit_test_type::outside;
            if( distance >= -radius ) result = hit_test_type::intersect;
        }
        return result;
    }

    /// Conservative six-plane culling with outward-facing planes, in the same space.
    /// Scaling contains half extents; x/y/z are the scaled box axes.
    /// inside: box strictly inside; outside: separated by a frustum plane;
    /// intersect: touching or potentially overlapping, not an exact overlap test.
    static hit_test_type hit_test_frustum_obb(
        this_t::frustum_t const & frustum, this_t::obb_t const & box ) noexcept
    {
        auto result = hit_test_type::inside;
        auto const x = box.x();
        auto const y = box.y();
        auto const z = box.z();
        for( size_t i = 0; i < size_t( frustum_t::frustum_plane::num_planes ); ++i )
        {
            auto const & plane = frustum.get_plane( i );
            auto const normal = plane.xyz();
            type_t const distance = normal.dot( box.origin() ) + plane.w();
            type_t const radius = std::abs( normal.dot( x ) ) +
                std::abs( normal.dot( y ) ) + std::abs( normal.dot( z ) );
            if( distance > radius ) return hit_test_type::outside;
            if( distance >= -radius ) result = hit_test_type::intersect;
        }
        return result;
    }

    /// Frustum planes face outward; ray and frustum must be in the same space.
    /// Returns the first surface hit with t >= 0, or the exit when starting inside.
    /// Nonzero directions need not be normalized. t_out is the ray parameter
    /// (a distance for normalized directions), or zero on a miss.
    static hit_test_type hit_test_frustum_ray(
        this_t::frustum_t const & frustum, motor::math::ray3f_t const & ray,
        type_t & t_out ) noexcept
    {
        t_out = type_t( 0 );
        vec3_t const origin = ray.get_origin();
        vec3_t const direction = ray.get_direction();
        if( direction.length2() == type_t( 0 ) ) return hit_test_type::outside;

        type_t t_min = std::numeric_limits< type_t >::lowest();
        type_t t_max = std::numeric_limits< type_t >::max();
        bool_t inside = true;
        bool_t on_surface = false;

        // Clip the ray interval against the interior half-space of each plane.
        for( size_t i = 0; i < size_t( frustum_t::frustum_plane::num_planes ); ++i )
        {
            auto const & plane = frustum.get_plane( i );
            auto const normal = plane.xyz();
            type_t const distance = normal.dot( origin ) + plane.w();
            type_t const denominator = normal.dot( direction );
            inside = inside && distance <= type_t( 0 );
            on_surface = on_surface || distance == type_t( 0 );

            if( denominator == type_t( 0 ) )
            {
                if( distance > type_t( 0 ) ) return hit_test_type::outside;
                continue;
            }

            type_t const t = -distance / denominator;
            if( denominator < type_t( 0 ) )
            {
                if( t > t_min ) t_min = t;
            }
            else if( t < t_max ) t_max = t;

            if( t_min > t_max || t_max < type_t( 0 ) ) return hit_test_type::outside;
        }

        if( inside && on_surface ) return hit_test_type::intersect;
        t_out = t_min >= type_t( 0 ) ? t_min : t_max;
        return hit_test_type::intersect;
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

    /// The orientation must be orthonormal; scaling contains the half extents.
    /// Ray and box must be in the same space, with a nonzero ray direction.
    /// Returns the first surface hit with t >= 0, or the exit when starting inside.
    /// t_out is the ray parameter (a distance for normalized directions), or zero on a miss.
    static hit_test_type hit_test_obb_ray(
        this_t::obb_t const & box, motor::math::ray3f_t const & ray, type_t & t_out ) noexcept
    {
        t_out = type_t( 0 );

        vec3_t const p = vec3_t( ray.get_origin() ) - box.origin();
        vec3_t const d = ray.get_direction();
        if( d.length2() == type_t( 0 ) ) return motor::collide::hit_test_type::outside;

        type_t t_min = std::numeric_limits< type_t >::lowest();
        type_t t_max = std::numeric_limits< type_t >::max();
        bool_t inside = true;
        bool_t on_surface = false;

        // Intersect the ray intervals between each pair of local box planes.
        for( size_t i = 0; i < 3; ++i )
        {
            auto const axis = box.orientation().get_column( i );
            type_t const e = p.dot( axis );
            type_t const f = d.dot( axis );
            type_t const h = std::abs( box.scaling()[ i ] );
            inside = inside && std::abs( e ) <= h;
            on_surface = on_surface || std::abs( e ) == h;

            if( f == type_t( 0 ) )
            {
                if( std::abs( e ) > h ) return motor::collide::hit_test_type::outside;
                continue;
            }

            type_t t1 = ( -h - e ) / f;
            type_t t2 = ( h - e ) / f;
            if( t1 > t2 )
            {
                type_t const tmp = t1;
                t1 = t2;
                t2 = tmp;
            }
            if( t1 > t_min ) t_min = t1;
            if( t2 < t_max ) t_max = t2;
            if( t_min > t_max || t_max < type_t( 0 ) )
                return motor::collide::hit_test_type::outside;
        }

        if( inside && on_surface ) return motor::collide::hit_test_type::intersect;

        t_out = t_min >= type_t( 0 ) ? t_min : t_max;
        return motor::collide::hit_test_type::intersect;
    }
};
} // namespace motor::collide::c3d
