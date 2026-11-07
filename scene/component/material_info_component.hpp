// SPDX-FileCopyrightText: 2023-2026 Alex Link
// SPDX-License-Identifier: MIT

#pragma once

#include "icomponent.h"

#include "../global.h"

#include <motor/wire/slot/output_slot.h>
#include <motor/wire/slot/sheet.hpp>

namespace motor::scene
{

class material_info_component : public motor::scene::icomponent
{
    typedef icomponent base_t;
    motor_this_typedefs( material_info_component );

    motor::string_t _name;

    motor::graphics::variable_set_mtr_t _vs = motor::shared( motor::graphics::variable_set_t() );
    motor::graphics::wire_variable_output_bridge_t _bridge;

  public:

    material_info_component( motor::string_in_t name ) noexcept : _name( name ) 
    {
        _bridge = motor::graphics::wire_variable_output_bridge_t( motor::share( _vs ) ) ;
    }
    material_info_component( this_cref_t ) = delete;
    material_info_component( this_rref_t rhv ) noexcept
        : _name( std::move( rhv._name ) ), _vs( motor::move( rhv._vs ) ),
          _bridge( std::move( rhv._bridge ) )
    {
    }
    virtual ~material_info_component( void_t ) noexcept 
    {
        motor::release( motor::move( _vs ) ) ;
    }

  public:

    motor::string_cref_t name( void_t ) const noexcept
    {
        return _name;
    }

    motor::graphics::variable_set_mtr_t borrow_var_set( void_t ) noexcept
    {
        return _vs;
    }    

    motor::wire::outputs_ref_t outputs( void_t ) noexcept
    {
        return _bridge.borrow_outputs();
    }

    void_t update_bridge( void_t ) noexcept
    {
        _bridge.update_bindings();
    }
};
motor_typedef( material_info_component );

} // namespace motor::scene