// SPDX-FileCopyrightText: 2023-2026 Alex Link
// SPDX-License-Identifier: MIT

#pragma once

#include "transformation.hpp"

namespace motor::math::m3d
{
template < typename T >
struct trs
{
    motor::math::vector3< T > translation{ T( 0 ) };
    motor::math::quaternion4< T > rotation{ T( 1 ), T( 0 ), T( 0 ), T( 0 ) };
    motor::math::vector3< T > scale{ T( 1 ) };

    // With a unit quaternion, the resulting matrix is T * R * S.
    transformation< T > to_transformation() const noexcept
    {
        transformation< T > result;
        result.set_scale( scale );
        result.rotate_by_quaternion_fl( rotation );
        result.set_translation( translation );
        return result;
    }
};
motor_typedefs( trs< float_t >, trsf );
} // namespace motor::math::m3d

namespace motor::math
{
motor_typedefs( motor::math::m3d::trs< float_t >, trs3f );
}
