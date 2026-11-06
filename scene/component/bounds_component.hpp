
// SPDX-FileCopyrightText: 2023-2026 Alex Link
// SPDX-License-Identifier: MIT

#pragma once

#include "icomponent.h"
#include "../global.h"

#include <motor/collide/3d/transform.hpp>
#include <motor/gfx/util/bounding_volume.hpp>

#include <motor/math/utility/3d/transformation.hpp>

#include <variant>

namespace motor::scene
{
class bounds_component : public icomponent
{
    motor_this_typedefs( bounds_component );

  public:

    using coarse_volume_t = motor::collide::aabb3f_t;

  private:

    coarse_volume_t _coarse;
    coarse_volume_t _coarse_transformed;

  public:

    bounds_component( void_t ) noexcept {}
    bounds_component( motor::collide::aabb3f_t const & bv ) noexcept : _coarse( bv ) {}
    bounds_component( this_cref_t rhv ) noexcept = delete;
    bounds_component( this_rref_t rhv ) noexcept
        : _coarse( std::move( rhv._coarse ) ),
          _coarse_transformed( std::move( rhv._coarse_transformed ) )
    {
    }
    virtual ~bounds_component( void_t ) noexcept {}

    void_t set_coarse( coarse_volume_t const & v ) noexcept
    {
        _coarse = v;
    }

    coarse_volume_t const & get_coarse( void_t ) const noexcept
    {
        return _coarse;
    }

    //
    void_t set_coarse_transformed( coarse_volume_t const & other ) noexcept
    {
        _coarse_transformed = other;
    }

    // @pre call transform somewhere first
    coarse_volume_t const & get_coarse_transformed( void_t ) noexcept
    {
        return _coarse_transformed;
    }

    void_t transform( motor::math::trafo3f_cref_t t ) noexcept
    {
        auto const obb = motor::collide::c3d::transform< float_t >::from_aabb( _coarse, t );
        _coarse_transformed = motor::collide::c3d::transform_bounds< float_t >::to_aabb( obb );
    }
};
motor_typedef( bounds_component );

} // namespace motor::scene