// SPDX-FileCopyrightText: 2023-2026 Alex Link
// SPDX-License-Identifier: MIT

#pragma once

#include "../../typedefs.h"

#include "../../vector/vector4.hpp"
#include "../../vector/vector3.hpp"
#include "../../matrix/matrix4.hpp"

#include <array>
#include <cmath>
#include <limits>

namespace motor::math::m3d
{

template < typename type_t >
class frustum
{
  public:

    motor_this_typedefs( frustum< type_t > );

    motor_typedefs( motor::math::vector4< type_t >, plane );

    using mat4_t = motor::math::matrix4< type_t >;
    using mat4_cref_t = mat4_t const &;

    using vec4_t = motor::math::vector4< type_t >;
    using vec3_t = motor::math::vector3< type_t >;

  private:

    plane_t _planes[ 6 ];

  public:

    enum class frustum_plane
    {
        left,
        right,
        bottom,
        top,
        near,
        far,
        num_planes
    };

  public:

    frustum( void_t ) noexcept {}
    frustum( this_t::mat4_cref_t view, this_t::mat4_cref_t proj ) noexcept 
    {
        this_t::contruct_planes_from( view, proj ) ;
    }

    this_ref_t set_plane( this_t::frustum_plane const fp, plane_cref_t plane )
    {
        _planes[ size_t( fp ) % size_t( frustum_plane::num_planes ) ] = plane;
        return *this;
    }

    plane_cref_t get_plane( this_t::frustum_plane const fp ) const
    {
        return _planes[ size_t( fp ) % size_t( frustum_plane::num_planes ) ];
    }

    plane_cref_t get_plane( size_t i ) const
    {
        return _planes[ i % size_t( frustum_plane::num_planes ) ];
    }

    /// Constructs outward-facing unit-normal planes from a [-w, +w] clip-Z projection.
    /// Interior points satisfy n.dot( p ) + d <= 0. Invalid planes are set to zero.
    void_t contruct_planes_from( mat4_cref_t view, mat4_cref_t proj ) noexcept
    {
        mat4_t const comb = proj * view;

        auto const r0 = comb.row( 0 );
        auto const r1 = comb.row( 1 );
        auto const r2 = comb.row( 2 );
        auto const r3 = comb.row( 3 );

        plane_t const planes[ 6 ] = { r3 + r0, r3 - r0, r3 + r1,
            r3 - r1, r3 + r2, r3 - r2 };
        for( size_t i = 0; i < 6; ++i )
        {
            type_t const length = planes[ i ].xyz().length();
            if( !( length > type_t( 0 ) ) || !std::isfinite( length ) ||
                !std::isfinite( planes[ i ].w() ) )
            {
                _planes[ i ] = plane_t( type_t( 0 ) );
                continue;
            }
            _planes[ i ] = planes[ i ] / -length;
        }
    }

    /// Near corners 0-3, far corners 4-7: bottom-left, top-left, top-right, bottom-right.
    /// Returns false without changing out if a corner has no finite intersection.
    bool_t points( std::array< vec3_t, 8 > & out ) const noexcept
    {
        using fp_t = this_t::frustum_plane;
        fp_t const corners[ 8 ][ 3 ] = {
            { fp_t::left, fp_t::bottom, fp_t::near },
            { fp_t::left, fp_t::top, fp_t::near },
            { fp_t::right, fp_t::top, fp_t::near },
            { fp_t::right, fp_t::bottom, fp_t::near },
            { fp_t::left, fp_t::bottom, fp_t::far },
            { fp_t::left, fp_t::top, fp_t::far },
            { fp_t::right, fp_t::top, fp_t::far },
            { fp_t::right, fp_t::bottom, fp_t::far }
        };

        std::array< vec3_t, 8 > result;
        for( size_t i = 0; i < result.size(); ++i )
        {
            auto const & a = this_t::get_plane( corners[ i ][ 0 ] );
            auto const & b = this_t::get_plane( corners[ i ][ 1 ] );
            auto const & c = this_t::get_plane( corners[ i ][ 2 ] );
            motor::math::vector3< double_t > const n0( a.xyz() ), n1( b.xyz() ), n2( c.xyz() );
            auto const n12 = n1.crossed( n2 );
            double_t const determinant = n0.dot( n12 );
            double_t const scale = n0.length() * n1.length() * n2.length();
            if( !std::isfinite( determinant ) || !std::isfinite( scale ) ||
                std::abs( determinant ) <= std::numeric_limits< double_t >::epsilon() * scale )
                return false;

            // Intersection of three planes with n.dot( p ) + d = 0.
            auto const point = ( n12 * -double_t( a.w() ) +
                n2.crossed( n0 ) * -double_t( b.w() ) +
                n0.crossed( n1 ) * -double_t( c.w() ) ) / determinant;
            for( size_t j = 0; j < 3; ++j )
            {
                if( !std::isfinite( point[ j ] ) ||
                    std::abs( point[ j ] ) > double_t( std::numeric_limits< type_t >::max() ) )
                    return false;
            }
            result[ i ] = vec3_t( point );
        }

        out = result;
        return true;
    }
};



} // namespace motor::math::m3d

namespace motor::math
{
    motor_typedefs( m3d::frustum< float_t >, frustum3f );
}
