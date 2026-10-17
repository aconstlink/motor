// SPDX-FileCopyrightText: 2023-2026 Alex Link
// SPDX-License-Identifier: MIT

#pragma once

#include "icomponent.h"
#include <motor/gfx/util/light.hpp>

namespace motor
{
namespace scene
{
class light_component : public icomponent
{
    motor_this_typedefs( light_component );

  private:

    motor::vector< motor::gfx::light_mtr_t > _lights;

  public:

    light_component( void_t ) noexcept{}
    light_component( motor::gfx::light_mtr_safe_t l ) noexcept
    {
        this_t::add_light( motor::move( l ) );
    }

    light_component( this_cref_t rhv ) noexcept = delete;
    light_component( this_rref_t rhv ) noexcept : _lights( std::move( rhv._lights ) ) {}
    virtual ~light_component( void_t ) noexcept
    {
        for( auto * light : _lights )
        {
            motor::release( motor::move( light ) );
        }
    }

    std::pair< bool_t, motor::gfx::light_mtr_safe_t > get_light( size_t const i ) const noexcept
    {
        if( _lights.size() <= i ) return std::make_pair( false, nullptr );
        return std::make_pair( true, motor::share( _lights[ i ] ) );
    }

    std::pair< bool_t, motor::gfx::light_mtr_t > borrow_light( size_t const i ) const noexcept
    {
        if( _lights.size() <= i ) return std::make_pair( false, nullptr );
        return std::make_pair( true, _lights[ i ] );
    }

    motor::gfx::light_type get_light_type( size_t const i ) const noexcept
    {
        if( _lights.size() > i ) return _lights[ i ]->get_light_type();
        return motor::gfx::light_type::invalid;
    }

    void_t add_light( motor::gfx::light_mtr_safe_t l ) noexcept
    {
        _lights.emplace_back( motor::move( l ) );
    }

    using for_each_borrowed_light_funk_t = std::function< void_t ( motor::gfx::light_mtr_t ) > ;
    void_t for_each_light( for_each_borrowed_light_funk_t f )
    {
        for( auto * l : _lights ) f( l ) ;
    }

    // use:
    // auto [a, b] = comp->borrow_light<...>() ;
    template < typename T >
    std::pair< bool_t, T * > borrow_cast_light( size_t const i ) const noexcept
    {
        T * ptr = dynamic_cast< T * >( _lights[ i ] );
        if( _lights.size() > i && ptr != nullptr )
        {
            return std::make_pair( true, ptr );
        }

        return std::make_pair( false, nullptr );
    }

    template < typename T >
    std::pair< bool_t, motor::core::mtr_safe< T > > get_cast_light( size_t const i ) const noexcept
    {
        T * ptr = dynamic_cast< T * >( _lights[ i ] );
        if( _lights.size() > i && ptr != nullptr )
        {
            return std::make_pair( true, motor::share( ptr ) );
        }

        return std::make_pair( false, nullptr );
    }

    void_t apply_transformation( motor::math::m3d::trafof_cref_t trafo ) noexcept
    {
        
    }
};
motor_typedef( light_component );
} // namespace scene
} // namespace motor