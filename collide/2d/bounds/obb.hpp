// SPDX-FileCopyrightText: 2023-2026 Alex Link
// SPDX-License-Identifier: MIT

#pragma once

#include "../../typedefs.h"

#include <motor/math/vector/vector2.hpp>
#include <motor/math/vector/vector4.hpp>

#include <motor/math/matrix/matrix2.hpp>

#include <motor/math/common/function.h>

namespace motor::collide::c2d
{
template < typename type_t >
class obb
{
    typedef aabb< type_t > this_t;
    typedef this_t & this_ref_t;
    typedef this_t const & this_cref_t;

    typedef motor::math::vector2< type_t > vec2_t;
    typedef vec2_t const & vec2_cref_t;
    typedef vec2_t & vec2_ref_t;
    typedef motor::math::vec2b_t vec2b_t;

    typedef motor::math::vector3< type_t > vec3_t;
    typedef vec3_t & vec3_ref_t;

    typedef motor::math::vector4< type_t > vec4_t;
    typedef motor::math::vec4b_t vec4b_t;

    typedef motor::math::matrix2< type_t > mat2_t;

  private:

    /// the translation to the center point
    vec2_t _center;

    /// the base of this obb
    /// [0] : u axis
    /// [1] : v axis
    vec2_t _base[ 2 ];

    // multiplies with the base
    vec2_t _extend;

  public:

    vec2_cref_t get_center( void_t ) const
    {
        return _center;
    }
    vec2_cref_t get_base_u( void_t ) const
    {
        return _base[ 0 ];
    }
    vec2_cref_t get_base_v( void_t ) const
    {
        return _base[ 1 ];
    }
    vec2_cref_t get_extend( void_t ) const
    {
        return _extend;
    }

  public:

    this_ref_t translate_to( vec2_cref_t new_pos )
    {
        _center = new_pos;
        return *this;
    }

    this_ref_t translate_by( vec2_cref_t translation )
    {
        return translate_to( _center + translation );
    }

    /// rotates the obb about the z axis.
    this_ref_t rotate( type_t angle )
    {
        const vec2_t cos_sin( angle, vec2_t::cos_sin );
        const mat2_t rot( cos_sin, mat2_t::rotation_matrix );

        _base[ 0 ] = rot * _base[ 0 ];
        _base[ 1 ] = rot * _base[ 1 ];

        return *this;
    }
};

} // namespace motor::collide::c2d
