//------------------------------------------------------------
// snakeoil (c) Alexis Constantin Link
// Distributed under the MIT license
//------------------------------------------------------------
#pragma once

#include "../../typedefs.h"

#include <motor/math/vector/vector2.hpp>
#include <motor/math/vector/vector4.hpp>
#include <motor/math/common/function.h>

namespace motor::collide::c2d
{
template < typename T >
class aabb
{
    motor_this_typedefs( aabb< T > );

    motor_typedefs( T, type );
    motor_typedefs( motor::math::vector2< type_t >, vec2 );
    motor_typedefs( motor::math::vec2b_t, vec2b );

    motor_typedefs( motor::math::vector3< type_t >, vec3 );
    motor_typedefs( motor::math::vector4< type_t >, vec4 );

  private:

    vec2_t _max;
    vec2_t _min;

  public:

    aabb( void_t ) {}

    /// constructs the aabb without checking for
    /// validity of the given values. The user must
    /// assert the correctness of max and being on the
    /// correct sides herself.
    aabb( vec2_cref_t min, vec2_cref_t max )
    {
        _max = max;
        _min = min;
    }

    /// constructs the aabb a circle like
    /// given parameters.
    aabb( vec2_cref_t center, type_t radius )
    {
        const vec2_t vmin = center - vec2_t( radius );
        const vec2_t vmax = center + vec2_t( radius );

        _max = vmax;
        _min = vmin;
    }

    aabb( this_cref_t rhv )
    {
        ( *this ) = rhv;
    }

  public: // test

    /// is point inside the aabb.
    bool_t is_inside( vec2_cref_t p ) const
    {
        vec2b_t const test_max = p.less_than( _max );
        vec2b_t const test_min = p.greater_than( _min );

        return test_max.all() && test_min.all();
    }

    bool_t is_inside( this_cref_t other ) const
    {
        bool_t inside = false;

        vec2_t points[ 4 ];
        other.get_points( points );

        for( size_t i = 0; i < 4; ++i )
        {
            inside = this_t::is_inside( points[ i ] );
            if( motor::core::is_not( inside ) ) break;
        }

        return inside;
    }

    /// computes the closest point to p on aabb.
    /// q will contain the closest point.
    /// @note only works if p is outside of aabb.
    /// otherwise, p itself is the closest point.
    void_t closest_point_to( vec2_cref_t p, vec2_ref_t q ) const
    {
        q = _min.max_ed( p );
        q = _max.min_ed( q );
    }

    /// returns the closest point to p.
    /// @see closest_point_to
    vec2_t closest_point_to( vec2_cref_t p ) const
    {
        vec2_t q;
        this_t::closest_point_to( p, q );
        return q;
    }

    /// returns the squared distance to the given point p
    /// from the aabb's bounds.
    /// @note if point is inside, 0 will be returned.
    type_t squared_distance_to( vec2_cref_t p ) const
    {
        const vec2b_t bmin = p.less_than( _min );
        const vec2b_t bmax = p.greater_than( _max );

        const vec2_t min_dist = _min - p;
        const vec2_t max_dist = p - _max;

        const vec4_t dist =
            vec4_t( min_dist, max_dist ).select( vec4_t( type_t( 0 ) ), vec4b_t( bmin, bmax ) );

        return dist.dot( dist );
    }

    /// Calculates the nearest aabb side normal with distance.
    /// @param p [in] the point to which the normal is required.
    /// @param n [out] the normal ( x, y, hesse distance )
    /// @note the distance is measured to be of length to the aabb's center.
    void_t calculate_normal_for( vec2_cref_t p, vec3_ref_t n ) const
    {
        const vec3_t p_local( p - this_t::get_center(), type_t( 1 ) );
        const vec2_t ext = this_t::get_extend();

        const type_t sign_x = motor::math::sign< type_t >( p_local.x() );
        const type_t sign_y = motor::math::sign< type_t >( p_local.y() );

        const vec3_t n_x( sign_x, type_t( 0 ), -ext.x() );
        const vec3_t n_y( type_t( 0 ), sign_y, -ext.y() );

        const type_t d_x = p_local.dot( n_x );
        const type_t d_y = p_local.dot( n_y );

        type_t const bias = d_x * d_y;

        // fully inside/outside
        if( bias > type_t( 0 ) )
        {
            vec2_t const dir_ext = ext * vec2_t( sign_x, sign_y );
            vec2_t const local_n = ( p_local.xy() - dir_ext ).normalized();
            n = vec3_t( local_n, -local_n.dot( dir_ext ) );
        }
        // outside partial
        else
        {
            n = ( d_x > type_t( 0 ) ) ? n_x : n_y;
        }
    }

    /// returns the aabb side normal for the point p.
    /// the normal will also contain the distance to the
    /// box's center.
    /// @see calculate_normal_for
    vec3_t calculate_normal_for( vec2_cref_t p ) const
    {
        vec3f_t n;
        calculate_normal_for( p, n );
        return n;
    }

  public: // transformation

    this_ref_t translate( vec2_cref_t translate )
    {
        this->set_min( _min + translate );
        this->set_max( _max + translate );
        return *this;
    }

    this_t translated( vec2_cref_t translate ) const
    {
        return this_t( *this ).translate();
    }

    void translate( vec2_cref_t translate, this_ref_t box_out ) const
    {
        box_out.set_max( this_t::get_max() + translate );
        box_out.set_min( this_t::get_min() + translate );
    }

    this_ref_t translate_to_position( vec2_t const & position )
    {
        vec2_t center = center();
        this_t::set_min( this_t::get_min() - center + position );
        this_t::set_max( this_t::get_max() - center + position );
        return *this;
    }

  public:

    this_ref_t operator()( vec2_cref_t max, vec2_cref_t min )
    {
        _max = max;
        _min = min;
        return *this;
    }

    this_ref_t operator=( this_cref_t rhv )
    {
        return ( *this )( rhv.get_max(), rhv.get_min() );
    }

  public:

    vec2_cref_t get_max( void_t ) const
    {
        return _max;
    }
    vec2_cref_t get_min( void_t ) const
    {
        return _min;
    }
    vec2_ref_t get_max( void_t )
    {
        return _max;
    }
    vec2_ref_t get_min( void_t )
    {
        return _min;
    }

    void_t set_max( vec2_cref_t max )
    {
        _max = max;
    }
    void_t set_min( vec2_cref_t min )
    {
        _min = min;
    }

    vec2_t get_center( void_t ) const
    {
        return ( this_t::get_max() - this_t::get_min() ) * type_t( 0.5 ) + this_t::get_min();
    }

    vec2_t get_dimensions( void_t ) const
    {
        return this_t::get_max() - this_t::get_min();
    }

    vec2_t get_extend( void_t ) const
    {
        return this_t::get_dimensions() * type_t( 0.5 );
    }

    type_t get_radius( void_t ) const
    {
        return this_t::get_extend().length();
    }

    type_t get_radius2( void_t ) const
    {
        return this_t::get_extend().length2();
    }

    // left bottom
    // left top
    // right top
    // right bottom
    void_t get_points( vec2_ptr_t points_out_ptr ) const
    {
        points_out_ptr[ 0 ] = vec2_t( _min.x(), _min.y() );
        points_out_ptr[ 1 ] = vec2_t( _min.x(), _max.y() );
        points_out_ptr[ 2 ] = vec2_t( _max.x(), _max.y() );
        points_out_ptr[ 3 ] = vec2_t( _max.x(), _min.y() );
    }

  public:

    typename this_t::vec2_t compute_relative( typename this_t::vec2_cref_t point ) const
    {
        return ( point - _min ) / this_t::get_dimensions();
    }
};

motor_typedefs( aabb< float_t >, aabbf );
} // namespace motor::collide::c2d
