// SPDX-FileCopyrightText: 2023-2026 Alex Link
// SPDX-License-Identifier: MIT

#pragma once

#include <motor/math/primitive/3d/frustum.hpp>
#include <motor/math/planar/plane/hesse_plane3.hpp>
#include <motor/math/matrix/matrix4.hpp>
#include <motor/math/vector/vector4.hpp>

namespace motor::collide::c3d
{
/*
    DO NOT USE!
*/
template < typename type_t >
class view_frustum
{
    typedef matrix< type_t, 4, 4 > mat4_t;
    typedef mat4_t const & mat4_cref_t;

    typedef vector< type_t, 4 > vec4_t;
    typedef vector< type_t, 3 > vec3_t;
    typedef vec3_t const & vec3_cref_t;

    typedef motor::collide::c3d::frustum< type_t > frustum_t;
    typedef frustum_t const & frustum_cref_t;

  public:

    typedef typename frustum_t::plane_t plane_t;
    typedef plane_t & plane_ref_t;

  private:

    frustum_t _frustum;

  public:

    view_frustum( void_t ) {}

  public:

    /// constructs this view frustum's planes from passed parameters.
    void_t contruct_planes_from( mat4_cref_t view, mat4_cref_t proj )
    {
        mat4_t comb = proj * view;

        vec4_t r0, r1, r2, r3;

        comb.get_row( 0, r0 );
        comb.get_row( 1, r1 );
        comb.get_row( 2, r2 );
        comb.get_row( 3, r3 );

        vec4_t p0 = ( r3 + r0 ).normalized();
        vec4_t p1 = ( r3 - r0 ).normalized();
        vec4_t p2 = ( r3 + r1 ).normalized();
        vec4_t p3 = ( r3 - r1 ).normalized();
        vec4_t p4 = ( r3 + r2 ).normalized();
        vec4_t p5 = ( r3 - r2 ).normalized();

        /*
        _planes[p_left]	= plane_t( p0.negate() ) ;
        _planes[p_right]	= plane_t( p1.negate() ) ;
        _planes[p_bottom] = plane_t( p2.negate() ) ;
        _planes[p_top]	= plane_t( p3.negate() ) ;
        _planes[p_near]	= plane_t( p4.negate() ) ;
        _planes[p_far]	= plane_t( p5.negate() );
        */

        _frustum.set_plane( frustum_t::p_left, plane_t( p0.negate() ) );
        _frustum.set_plane( frustum_t::p_right, plane_t( p1.negate() ) );
        _frustum.set_plane( frustum_t::p_bottom, plane_t( p2.negate() ) );
        _frustum.set_plane( frustum_t::p_top, plane_t( p3.negate() ) );
        _frustum.set_plane( frustum_t::p_near, plane_t( p4.negate() ) );
        _frustum.set_plane( frustum_t::p_far, plane_t( p5.negate() ) );
    }

    frustum_cref_t get_frustum( void_t ) const
    {
        return _frustum;
    }
};

} // namespace motor::collide::c3d
