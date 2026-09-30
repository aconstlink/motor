
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

    motor::gfx::light_mtr_t _light = nullptr;

  public:

    light_component( motor::gfx::light_mtr_safe_t l ) noexcept : _light( l ) {}
    light_component( this_cref_t rhv ) noexcept = delete;
    light_component( this_rref_t rhv ) noexcept : _light( motor::move( rhv._light ) ) {}
    virtual ~light_component( void_t ) noexcept {}

    motor::gfx::light_mtr_t get_light( void_t ) const noexcept
    {
        return _light;
    }

    motor::gfx::light_type get_light_type( void_t ) const noexcept
    {
        if( _light ) return _light->get_light_type();
        return motor::gfx::light_type::invalid;
    }

    // use:
    // auto [a, b] = comp->borrow_light<...>() ;
    template < typename T >
    std::pair< bool_t, T * > borrow_light( void_t ) const noexcept
    {
        if( auto * ptr = dynamic_cast< T * >( _light ); ptr != nullptr )
        {
            return std::make_pair( true, ptr );
        }

        return std::make_pair( false, nullptr );
    }

    template < typename T >
    std::pair< bool_t, motor::core::mtr_safe<T> > get_light( void_t ) const noexcept
    {
        if( auto * ptr = dynamic_cast< T * >( _light ); ptr != nullptr )
        {
            return std::make_pair( true, motor::share( ptr ) );
        }

        return std::make_pair( true, nullptr );
    }
};
motor_typedef( light_component );
} // namespace scene
} // namespace motor