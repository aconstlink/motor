
#pragma once

#include "icomponent.h"

#include "icontroller.h"

#include <motor/io/database.h>
#include <motor/io/monitor.hpp>

namespace motor
{
namespace scene
{
class shader_change_observer_component : public icomponent
{
    motor_this_typedefs( shader_change_observer_component );

    public:

    enum class state
    {
        invalid,
        no_change,
        in_transit,
        changed
    };
  private:

    motor::io::monitor_mtr_t _file_mon = motor::shared( motor::io::monitor_t() );
    motor::io::database_mtr_t _db = nullptr;
    motor::wire::funk_node_mtr_t _react_to_file_change = nullptr ;
    this_t::state _loading_state = state::invalid ;

    motor::string_t _shader  ;

  public:

    shader_change_observer_component( motor::io::database_mtr_safe_t db ) noexcept;
    shader_change_observer_component( motor::io::database_mtr_safe_t db, motor::io::location_cref_t ) noexcept;
    shader_change_observer_component( this_cref_t rhv ) noexcept = delete;
    shader_change_observer_component( this_rref_t rhv ) noexcept;
    virtual ~shader_change_observer_component( void_t ) noexcept;

    void_t monitor_location( motor::io::location_cref_t ) noexcept;

    state has_file_changed( void_t ) noexcept;

  private:

    void_t create_observer_funk( void_t ) noexcept;
};
motor_typedef( shader_change_observer_component );
} // namespace scene
} // namespace motor