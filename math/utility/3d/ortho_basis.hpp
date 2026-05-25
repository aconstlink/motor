#pragma once

#include "../../vector/vector3.hpp"
#include "../../matrix/matrix3.hpp"
#include "../../matrix/matrix4.hpp"

#include <limits>

namespace motor
{
namespace math
{
namespace m3d
{
// creates right handed orthonormal basis.
template < typename T >
class orthonormal_basis
{
    using this_t = orthonormal_basis< T >;
    using real_t = T;
    using vec3_t = motor::math::vector3< T >;
    using mat3_t = motor::math::matrix3< T >;
    using mat4_t = motor::math::matrix4< T >;

  public:

    static void create( vec3_t const & dir, vec3_t const & sug_up, vec3_t & x, vec3_t & y, vec3_t & z ) noexcept
    {
        z = dir;

        vec3_t up = sug_up;

        // This check must be done.
        // If the normal and the desired up vector is colinear,
        // another up vector must be choosen.
        if( T(1) - std::abs( up.dot( dir ) ) <=  std::numeric_limits<T>::min() )
            up = vec3_t( 0.0f, 0.0f, 1.0f );

        y = ( up - ( dir * ( dir.dot( up ) ) ) ).normalize();
        x = y.crossed( dir );
    }

    static void create( vec3_t const & dir, vec3_t & x, vec3_t & y, vec3_t & z ) noexcept
    {
        return this_t::create( dir, vec3f_t(0.0f,1.0f,0.0f), x, y, z ) ;
    }

    /**
        This function creates an orthonormal basis by using a given direction
        vector. This is the normal vector.
        This basis is than normalized and orthogonal.

        Be sure to not pass a null vector. There is no checking.

        @param normal [in] A non null vector. Must be normalized.
        @param matOut [out] The created orthonormal basis.
    */
    static void create( vec3_t const & dir, mat3_t & matOut ) noexcept
    {
        if( dir.length2() <= std::numeric_limits< vec3_t::type_t >::min() ) return;

        vec3_t vcX;
        vec3_t vcY;
        vec3_t vcZ;

        this_t::create( dir, vcX, vcY, vcZ );

        matOut.set_column( 0, vcX );
        matOut.set_column( 1, vcY );
        matOut.set_column( 2, vcZ );
    }

    // creates an affine matrix with orthonormal frame and position stuffed in 4th 
    // column vector. in order to make it usefull, mat[15] = 1.0f
    // @param sub_up a suggested up vector.
    static void create_affine( vec3_t const & pos, vec3_t const sug_up, vec3_t const & dir, mat4_t & mat ) noexcept
    {
        if( dir.length2() <= std::numeric_limits< vec3_t::type_t >::min() ) return;

        vec3_t x;
        vec3_t y;
        vec3_t z;

        this_t::create( dir, sug_up, x, y, z );

        mat.set_column( 0, x );
        mat.set_column( 1, y );
        mat.set_column( 2, z );
        mat.set_column( 3, pos );

        mat[15] = 1.0f ;
    }

    // creates an affine matrix with orthonormal frame and position stuffed in 4th 
    // column vector. in order to make it usefull, mat[15] = 1.0f
    static void create_affine( vec3_t const & pos, vec3_t const & dir, mat4_t & mat ) noexcept
    {
        return this_t::create_affine( pos, vec3_t(0.0f,1.0f,0.0f), dir, mat ) ;
    }
};

} // namespace m3d
} // namespace math
} // namespace motor