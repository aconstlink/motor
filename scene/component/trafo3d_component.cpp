// SPDX-FileCopyrightText: 2023-2026 Alex Link
// SPDX-License-Identifier: MIT

#include "../global.h"
#include "trafo3d_component.h"

using namespace motor::scene;

uint64_t const trafo3d_component::tid =
    motor::scene::global::register_internal_type( "motor::scene::trafo3d_component" );

//********************************************************************************
trafo3d_component::trafo3d_component( void_t ) noexcept
{
    this_t::init_slots();
}

//********************************************************************************
trafo3d_component::trafo3d_component( motor::math::m3d::trafof_cref_t trafo_local ) noexcept
    : _trafo_local( trafo_local )
{
    this_t::init_slots();
    // no local coord frame change!
    //_trafo_is->set_value();
}

//********************************************************************************
trafo3d_component::trafo3d_component( this_rref_t rhv ) noexcept
    : icomponent( std::move( rhv ) ), _trafo_is( motor::move( rhv._trafo_is ) ),
      _computed_os( motor::move( rhv._computed_os ) ), _composer( std::move( rhv._composer ) ),
      _trafo( std::move( rhv._trafo ) ), _trafo_local( std::move( rhv._trafo_local ) ),
      _computed( std::move( rhv._computed ) )
{
}

//********************************************************************************
trafo3d_component::~trafo3d_component( void_t ) noexcept
{
    // icomponent should disconnect and release the slots.
    // so we do not do it here.
    // if( _trafo_is ) _trafo_is->disconnect();
    // if( _computed_os ) _computed_os->disconnect();

    for( auto & var : _composer )
    {
        // the owner needs to disconnect, otherwise
        // there will be some dangling node pointers.

        this_t::disconnect( var );
        this_t::release( var );
    }
}

//********************************************************************************
void_t trafo3d_component::disconnect( composer_variant_t & var ) const noexcept
{
    std::visit( []( auto && arg )
    {
        using T = std::decay_t< decltype( arg ) >;
        if constexpr( !std::is_same_v< T, std::nullptr_t > )
        {
            arg->disconnect();
        }
    }, var );
}

//********************************************************************************
void_t trafo3d_component::release( composer_variant_t & var ) const noexcept
{
    std::visit( []( auto && arg )
    {
        using T = std::decay_t< decltype( arg ) >;
        if constexpr( !std::is_same_v< T, std::nullptr_t > )
        {
            motor::release( motor::move( arg ) );
        }
    }, var );
}

//**********************************************************************************
bool_t trafo3d_component::inputs( motor::wire::inputs_out_t ins ) noexcept
{
    ins = motor::wire::inputs_t( { { "trafo", motor::share( this_t::_trafo_is ) } }, false );
    return true;
}

//**********************************************************************************
void_t trafo3d_component::init_slots( void_t ) noexcept
{
    if( _trafo_is != nullptr ) return;

    _trafo_is =
        this_t::create_input_slot< motor::math::m3d::trafof_t >( "[trafo3d_component] : trafo_is" );
    _computed_os = this_t::create_output_slot< motor::math::m3d::trafof_t >(
        "[trafo3d_component] : trafo_os" );
}

//**********************************************************************************
void_t trafo3d_component::sync_inputs( void_t ) noexcept
{
    if( _trafo_is->has_changed() )
        _trafo = _trafo_local * _trafo_is->pull_data();
    else
        _trafo = _trafo_local;
}

//**********************************************************************************
void_t trafo3d_component::sync_outputs( void_t ) noexcept {}