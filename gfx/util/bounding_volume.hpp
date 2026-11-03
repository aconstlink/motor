// SPDX-FileCopyrightText: 2023-2026 Alex Link
// SPDX-License-Identifier: MIT

#pragma once

#include "../typedefs.h"

#include <array>
#include <motor/collide/3d/aabb.hpp>
#include <motor/collide/3d/obb.hpp>
#include <motor/collide/3d/sphere.hpp>
#include <motor/collide/3d/transform.hpp>

namespace motor
{
namespace gfx
{
namespace volumes
{
enum class volume_type
{
    aabb,
    obb,
    sphere
};

class bounding_volume
{
  public:

    virtual ~bounding_volume( void_t ) noexcept = default;

    virtual volume_type get_volume_type( void_t ) const noexcept = 0;
    virtual motor::collide::aabb3f_t enclosing_aabb( void_t ) const noexcept = 0;
};
motor_typedef( bounding_volume ) ;

class aabb : public bounding_volume
{
    motor_this_typedefs( aabb );

  private:

    motor::collide::aabb3f_t _bounds;

  public:

    aabb( void_t ) noexcept
        : _bounds( motor::math::vec3f_t( 0.0f ), motor::math::vec3f_t( 0.0f ) )
    {
    }

    explicit aabb( motor::collide::aabb3f_cref_t bounds ) noexcept : _bounds( bounds ) {}

    // Keep collide's (max, min) parameter order.
    aabb( motor::math::vec3f_cref_t max, motor::math::vec3f_cref_t min ) noexcept
        : _bounds( max, min )
    {
    }

    static volume_type get_volume_type_static( void_t ) noexcept
    {
        return volume_type::aabb;
    }

    volume_type get_volume_type( void_t ) const noexcept override
    {
        return this_t::get_volume_type_static();
    }

    motor::collide::aabb3f_cref_t get_bounds( void_t ) const noexcept
    {
        return _bounds;
    }

    motor::collide::aabb3f_t enclosing_aabb( void_t ) const noexcept override
    {
        return _bounds;
    }
};
motor_typedef( aabb );

class obb : public bounding_volume
{
    motor_this_typedefs( obb );

  private:

    motor::collide::obb3f_t _bounds;

  public:

    obb( void_t ) noexcept
        : _bounds( motor::math::vec3f_t( 0.0f ), motor::math::vec3f_t( 0.0f ),
              motor::math::mat3f_t::make_identity() )
    {
    }

    explicit obb( motor::collide::obb3f_cref_t bounds ) noexcept : _bounds( bounds ) {}

    obb( motor::math::vec3f_cref_t center, motor::math::vec3f_cref_t half_extents,
        motor::math::mat3f_cref_t orientation ) noexcept
        : _bounds( center, half_extents, orientation )
    {
    }

    static volume_type get_volume_type_static( void_t ) noexcept
    {
        return volume_type::obb;
    }

    volume_type get_volume_type( void_t ) const noexcept override
    {
        return this_t::get_volume_type_static();
    }

    motor::collide::obb3f_cref_t get_bounds( void_t ) const noexcept
    {
        return _bounds;
    }

    motor::collide::aabb3f_t enclosing_aabb( void_t ) const noexcept override
    {
        return motor::collide::c3d::transform_bounds< float_t >::to_aabb( _bounds );
    }
};
motor_typedef( obb );

class sphere : public bounding_volume
{
    motor_this_typedefs( sphere );

  private:

    motor::collide::sphere3f_t _bounds;

  public:

    sphere( void_t ) noexcept : _bounds( motor::math::vec3f_t( 0.0f ), 0.0f ) {}

    explicit sphere( motor::collide::sphere3f_cref_t bounds ) noexcept : _bounds( bounds ) {}

    sphere( motor::math::vec3f_cref_t center, float_t const radius ) noexcept
        : _bounds( center, radius )
    {
    }

    static volume_type get_volume_type_static( void_t ) noexcept
    {
        return volume_type::sphere;
    }

    volume_type get_volume_type( void_t ) const noexcept override
    {
        return this_t::get_volume_type_static();
    }

    motor::collide::sphere3f_cref_t get_bounds( void_t ) const noexcept
    {
        return _bounds;
    }

    motor::collide::aabb3f_t enclosing_aabb( void_t ) const noexcept override
    {
        return motor::collide::aabb3f_t( _bounds.get_center(), _bounds.get_radius() );
    }
};
motor_typedef( sphere );

} // namespace volumes

} // namespace gfx
} // namespace motor
