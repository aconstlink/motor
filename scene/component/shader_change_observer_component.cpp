
#pragma once

#include "shader_change_observer_component.h"

using namespace motor;
using namespace motor::scene;

//
// shader_change_observer_component
//

//*******************************************************************************************
shader_change_observer_component::shader_change_observer_component(
    motor::io::database_mtr_safe_t db ) noexcept
    : _db( motor::move( db ) )
{
    this_t::create_observer_funk();
}

//*******************************************************************************************
shader_change_observer_component::shader_change_observer_component(
    motor::io::database_mtr_safe_t db, motor::io::location_cref_t loc ) noexcept
    : _db( motor::move( db ) )
{
    this_t::create_observer_funk();
    this_t::monitor_location( loc );
}

//*******************************************************************************************
shader_change_observer_component::shader_change_observer_component( this_rref_t rhv ) noexcept
    : _file_mon( motor::move( rhv._file_mon ) ), _db( motor::move( rhv._db ) )
{
    this_t::create_observer_funk();
}

//*******************************************************************************************
shader_change_observer_component::~shader_change_observer_component( void_t ) noexcept
{
    motor::release( motor::move( _file_mon ) );
    motor::release( motor::move( _db ) );
}

//*******************************************************************************************
void_t shader_change_observer_component::monitor_location( motor::io::location_cref_t loc ) noexcept
{
    _db->detach( _file_mon );
    _db->attach( loc, motor::share( _file_mon ) );
}

//*******************************************************************************************
shader_change_observer_component::state shader_change_observer_component::has_file_changed(
    void_t ) noexcept
{
    if( _loading_state == this_t::state::in_transit ) return _loading_state;

    _file_mon->for_each_and_swap(
        [ & ]( motor::io::location_cref_t loc, motor::io::monitor_t::notify const n )
    {
        // load it.
        int bp = 0;
    } );
    return this_t::state::no_change;
}

//*******************************************************************************************
void_t shader_change_observer_component::create_observer_funk( void_t ) noexcept
{
    motor::release( motor::move( _react_to_file_change ) );

    auto node = motor::wire::funk_node_t( [ & ]( motor::wire::funk_node_ptr_t ) //
    {
        this->_file_mon->for_each_and_swap(
            [ & ]( motor::io::location_cref_t loc, motor::io::monitor_t::notify const n )
        {
            // load it.
        } );
    } );
    _react_to_file_change = motor::shared( std::move( node ) );
}