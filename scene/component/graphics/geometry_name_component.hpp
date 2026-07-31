
#pragma once

#include "../icomponent.h"

#include <motor/graphics/frontend/command_status.hpp>
#include <motor/graphics/object/geometry_object.h>
#include <motor/std/hash_map>

namespace motor
{
namespace scene
{
class geometry_name_component : public icomponent
{
    motor_this_typedefs( geometry_name_component );

  public:

  private:

    motor::string_t _name;

  public:

    geometry_name_component( this_rref_t rhv ) noexcept : _name( std::move( rhv._name ) ) {}
    geometry_name_component( this_cref_t ) = delete;
    geometry_name_component( motor::string_in_t name ) noexcept : _name( name ) {}

    virtual ~geometry_name_component( void_t ) noexcept {}

    motor::string_cref_t get_name( void_t ) const noexcept
    {
        return _name;
    }
};
motor_typedef( geometry_name_component );
} // namespace scene
} // namespace motor