#pragma once

#include "interpolate.hpp"

#include "../quaternion/quaternion4.hpp"

namespace motor
{
namespace math
{
template < typename T >
struct interpolation< motor::math::quaternion4< T > >
{
    motor_typedefs( motor::math::quaternion4< T >, value );
    typedef float_t real_t;

    // becomes slerp
    static value_t linear( value_cref_t p1, value_cref_t p2, real_t const & t ) noexcept
    {
        auto q0 = p1.normalized();
        auto q1 = p2.normalized();

        T d = q0.real() * q1.real() + q0.i() * q1.i() + q0.j() * q1.j() + q0.k() * q1.k();

        if( d < 0.0f )
        {
            q1 = value_t( -q1.real(), -q1.i(), -q1.j(), -q1.k() );
            d = -d;
        }

        if( d > 0.9995f )
        {
            value_t r( q0.real() * ( 1.0f - t ) + q1.real() * t,
                        q0.i() * ( 1.0f - t ) + q1.i() * t, q0.j() * ( 1.0f - t ) + q1.j() * t,
                        q0.k() * ( 1.0f - t ) + q1.k() * t );

            return r.normalize();
        }

        T theta0 = acosf( d );
        T theta = theta0 * t;
        T sin_theta0 = sinf( theta0 );

        T s0 = sinf( theta0 - theta ) / sin_theta0;
        T s1 = sinf( theta ) / sin_theta0;

        return value_t( q0.real() * s0 + q1.real() * s1, q0.i() * s0 + q1.i() * s1,
                         q0.j() * s0 + q1.j() * s1, q0.k() * s0 + q1.k() * s1 );
    }
};
} // namespace math
} // namespace motor