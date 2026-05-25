#pragma once

#include "../../vector/vector3.hpp"
#include "../../matrix/matrix3.hpp"

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

  public:

    static void create( vec3_t const & dir, vec3_t & outX, vec3_t & outY, vec3_t & outZ ) noexcept
    {
        outZ = dir;

        vec3_t up = vec3_t( 0.0f, 1.0f, 0.0f );

        // This check must be done.
        // If the normal and the desired up vector is colinear,
        // another up vector must be choosen.
        if( T(1) - std::abs( up.dot( dir ) ) <=  std::numeric_limits<T>::min() )
            up = vec3_t( 0.0f, 0.0f, 1.0f );

        outY = ( up - ( dir * ( dir.dot( up ) ) ) ).normalize();
        outX = outY.crossed( dir );
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
};

} // namespace m3d
} // namespace math
} // namespace motor