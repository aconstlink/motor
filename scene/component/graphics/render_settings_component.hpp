
#pragma once

#include "../icomponent.h"

#include <motor/graphics/frontend/command_status.hpp>
#include <motor/graphics/object/state_object.h>
#include <motor/std/hash_map>

namespace motor
{
namespace scene
{
class MOTOR_SCENE_API render_settings_component : public icomponent
{
    motor_this_typedefs( render_settings_component );

  public:

    // can accossiate a specific state_object to
    // an id. the id can be specified by the user.
    // although, id==0 should be the base render state.
    using id_t = size_t;

  private:

    struct data
    {
        motor::graphics::command_status_mtr_t status;
        motor::graphics::state_object_mtr_t state;
    };
    motor_typedef( data );

    using map_t = motor::hash_map< id_t, data_t >;
    map_t _rs;

  public:

    render_settings_component( this_rref_t rhv ) noexcept : _rs( std::move( rhv._rs ) ) {}
    render_settings_component( this_cref_t ) = delete;
    render_settings_component( motor::graphics::state_object_mtr_safe_t rs ) noexcept
    {
        _rs[ 0 ] = { motor::shared( motor::graphics::command_status_t() ), motor::move( rs ) };
    }

    render_settings_component( motor::graphics::render_state_sets_rref_t rs ) noexcept
    {
        _rs[ 0 ] = { motor::shared( motor::graphics::command_status_t() ),
            motor::shared( motor::graphics::state_object_t( std::move( rs ) ) ) };
    }

    virtual ~render_settings_component( void_t ) noexcept
    {
        for( auto i : _rs )
        {
            motor::release( motor::move( i.second.state ) );
            motor::release( motor::move( i.second.status ) );
        }
    }

    bool_t borrow_state( id_t const id,
        std::function< void_t(
            motor::graphics::command_status_mtr_t, motor::graphics::state_object_mtr_t ) >
            fn ) noexcept
    {
        auto iter = _rs.find( id );
        if( iter == _rs.end() ) return false;
        
        fn( iter->second.status, iter->second.state ) ;
        
        return true;
    }

    bool_t add_state( id_t const id, motor::graphics::state_object_mtr_safe_t rs ) noexcept
    {
        auto iter = _rs.find( id );
        if( iter != _rs.end() ) return false;
        _rs[ id ] = { motor::shared( motor::graphics::command_status_t() ), motor::move( rs ) };
        return true;
    }

    bool_t add_state( id_t const id, motor::graphics::render_state_sets_rref_t rs ) noexcept
    {
        auto iter = _rs.find( id );
        if( iter != _rs.end() ) return false;
        _rs[ id ] = { motor::shared( motor::graphics::command_status_t() ),
            motor::shared( motor::graphics::state_object_t( std::move( rs ) ) ) };
        return true;
    }
};
motor_typedef( render_settings_component );
} // namespace scene
} // namespace motor