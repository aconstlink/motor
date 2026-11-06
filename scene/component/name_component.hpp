// SPDX-FileCopyrightText: 2023-2026 Alex Link
// SPDX-License-Identifier: MIT

#pragma once

#include "icomponent.h"
#include <motor/std/string>

namespace motor
{
namespace scene
{
class MOTOR_SCENE_API name_component : public icomponent
{
    motor_this_typedefs( name_component );

  public:

    static uint64_t const tid;
    virtual uint64_t type_id( void_t ) const noexcept override
    {
        return tid;
    }

  private:

    motor::string_t _name;

  public:

    name_component( motor::string_cref_t n ) noexcept : _name( n ) {}
    name_component( this_cref_t rhv ) noexcept : icomponent(), _name( rhv._name ) {}
    name_component( this_rref_t rhv ) noexcept : _name( std::move( rhv._name ) ) {}
    virtual ~name_component( void_t ) noexcept {}

    motor::string_cref_t get_name( void_t ) const noexcept
    {
        return _name;
    }
};
motor_typedef( name_component );
} // namespace scene
} // namespace motor