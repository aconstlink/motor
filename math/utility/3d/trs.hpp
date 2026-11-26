// SPDX-FileCopyrightText: 2023-2026 Alex Link
// SPDX-License-Identifier: MIT

#pragma once

#include "transformation.hpp"

namespace motor::math::m3d
{
template < typename T >
struct trs
{
    motor_this_typedefs( trs );

    using translation_t = motor::math::vector3< T >;
    using rotation_t = motor::math::quaternion4< T >;
    using scale_t = motor::math::vector3< T >;

    translation_t translation{ T( 0 ) };
    rotation_t rotation{ T( 1 ), T( 0 ), T( 0 ), T( 0 ) };
    scale_t scale{ T( 1 ) };

    // With a unit quaternion, the resulting matrix is T * R * S.
    transformation< T > to_transformation() const noexcept
    {
        transformation< T > result;
        result.set_scale( scale );
        result.rotate_by_quaternion_fl( rotation );
        result.set_translation( translation );
        return result;
    }

    this_ref_t operator=( this_cref_t rhv ) noexcept
    {
        translation = rhv.translation;
        rotation = rhv.rotation;
        scale = rhv.scale;

        return *this;
    }
};
motor_typedefs( trs< float_t >, trsf );
} // namespace motor::math::m3d

namespace motor::math
{
motor_typedefs( motor::math::m3d::trs< float_t >, trs3f );
}
