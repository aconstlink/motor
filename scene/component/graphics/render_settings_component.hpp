
#pragma once

#include "../icomponent.h"

#include <motor/graphics/frontend/command_status.hpp>
#include <motor/graphics/object/state_object.h>
#include <motor/std/vector>

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
        id_t id;
        motor::graphics::state_object_mtr_t state;
    };
    motor_typedef( data );

    using states_t = motor::vector< data >;
    states_t _rs;

  public:

    render_settings_component( this_rref_t rhv ) noexcept : _rs( std::move( rhv._rs ) ) {}
    render_settings_component( this_cref_t ) = delete;
    render_settings_component( motor::graphics::state_object_mtr_safe_t rs ) noexcept
    {
        this_t::add_state( 0, motor::move( rs ) );
    }

    render_settings_component( motor::graphics::render_state_sets_rref_t rs ) noexcept
    {
        this_t::add_state( 0, motor::shared( motor::graphics::state_object_t( std::move( rs ) ) ) ) ;
    }

    virtual ~render_settings_component( void_t ) noexcept
    {
        for( auto & i : _rs )
        {
            motor::release( motor::move( i.state ) );
        }
    }

    bool_t borrow_state(
        id_t const id, std::function< void_t( motor::graphics::state_object_mtr_t ) > fn ) noexcept
    {
        size_t i = size_t( -1 );
        while( ++i < _rs.size() && _rs[ i ].id != id );
        if( i == _rs.size() ) return false;

        fn( _rs[ i ].state );

        return true;
    }

    bool_t add_state( id_t const id, motor::graphics::state_object_mtr_safe_t rs ) noexcept
    {
        // is the id already in the set
        if( this_t::has_id( id ) ) return false ;
        return this_t::add_state_no_check( id, motor::move( rs ) ) ;
    }

    #if 0
    bool_t add_state( id_t const id, motor::graphics::render_state_sets_rref_t rs ) noexcept
    {
        // add render state sets to the render state object.
        // I think multiple set are not supported per state object at the moment.
    }
    #endif
    
  private:

    // check if id already in the set
    bool_t has_id( id_t const id ) const noexcept
    {
        size_t i = size_t( -1 );
        while( ++i < _rs.size() && _rs[ i ].id != id );
        return i != _rs.size() ;
    }

    // does not check if id already in the set.
    // adds the passed render states in the set with id.
    bool_t add_state_no_check( id_t const id, motor::graphics::state_object_mtr_safe_t rs ) noexcept
    {
        // if not, search for empty spot
        size_t i = size_t( -1 );
        while( ++i < _rs.size() && _rs[ i ].id != size_t( -1 ) );
        if( i == _rs.size() )
        {
            _rs.emplace_back( data_t{ 0, motor::move( rs ) } );
            return true;
        }

        _rs[ i ] = data_t{ id, motor::move( rs ) };

        return true;
    }
};
motor_typedef( render_settings_component );
} // namespace scene
} // namespace motor