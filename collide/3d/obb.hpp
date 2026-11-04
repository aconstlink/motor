// SPDX-FileCopyrightText: 2023-2026 Alex Link
// SPDX-License-Identifier: MIT

#pragma once

#include "../typedefs.h"

#include <motor/math/vector/vector3.hpp>
#include <motor/math/utility/3d/transformation.hpp>

namespace motor::collide::c3d
{
// oriented bounding box
template < typename type_t >
class obb
{
    motor_this_typedefs( obb< type_t > );

    motor_typedefs( motor::math::vector3< type_t >, vec3 );
    motor_typedefs( motor::math::matrix3< type_t >, mat3 );

  private:

    this_t::vec3_t _origin;
    this_t::vec3_t _scaling;
    this_t::mat3_t _orientation;

  public:

    obb( void_t ) noexcept {}

    obb( this_cref_t rhv ) noexcept
        : _origin( rhv._origin ), _scaling( rhv._scaling ), _orientation( rhv._orientation )
    {
    }

    obb( this_rref_t rhv ) noexcept
        : _origin( rhv._origin ), _scaling( rhv._scaling ), _orientation( rhv._orientation )
    {
    }

    obb( this_t::vec3_cref_t origin, this_t::vec3_cref_t scaling,
        this_t::mat3_cref_t orientation ) noexcept
        : _origin( origin ), _scaling( scaling ), _orientation( orientation )
    {
    }

    this_t::vec3_cref_t origin( void_t ) const noexcept
    {
        return _origin;
    }

    this_t::vec3_cref_t scaling( void_t ) const noexcept
    {
        return _scaling;
    }

    this_t::mat3_cref_t orientation( void_t ) const noexcept
    {
        return _orientation;
    }

    this_t::vec3_t extend_x( void_t ) const noexcept
    {
        return this_t::vec3_t( _orientation.column(0) * _scaling[0] ) ;
    }

    this_t::vec3_t extend_y( void_t ) const noexcept
    {
        return this_t::vec3_t( _orientation.column(1) * _scaling[1] ) ;
    }

    this_t::vec3_t extend_z( void_t ) const noexcept
    {
        return this_t::vec3_t( _orientation.column(2) * _scaling[2] ) ;
    }

    this_t::vec3_t x( void_t ) const noexcept
    {
        return _orientation.get_column( 0 ) * _scaling.x();
    }

    this_t::vec3_t y( void_t ) const noexcept
    {
        return _orientation.get_column( 1 ) * _scaling.y();
    }

    this_t::vec3_t z( void_t ) const noexcept
    {
        return _orientation.get_column( 2 ) * _scaling.z();
    }

  public:

    std::array< this_t::vec3_t, 8 > points( void_t ) const noexcept
    {
        auto const x = _orientation.get_column( 0 ) * _scaling.x();
        auto const y = _orientation.get_column( 1 ) * _scaling.y();
        auto const z = _orientation.get_column( 2 ) * _scaling.z();

        return {
            // front
            _origin - x - y + z,
            _origin - x + y + z,
            _origin + x + y + z,
            _origin + x - y + z,
            // back
            _origin - x - y - z,
            _origin - x + y - z,
            _origin + x + y - z,
            _origin + x - y - z,
        };
    }
};

} // namespace motor::collide::c3d

namespace motor::collide
{
motor_typedefs( motor::collide::c3d::obb< float_t >, obb3f );
}