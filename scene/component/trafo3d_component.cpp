
#include "trafo3d_component.h"

using namespace motor::scene;

//********************************************************************************
trafo3d_component::trafo3d_component( void_t ) noexcept
{
    this_t::init_slots();
}

//********************************************************************************
trafo3d_component::trafo3d_component( motor::math::m3d::trafof_cref_t trafo ) noexcept : _trafo( trafo )
{
    this_t::init_slots();
    _trafo_is->set_value( trafo );
}

//********************************************************************************
trafo3d_component::trafo3d_component( this_rref_t rhv ) noexcept
    : icomponent( std::move( rhv ) ), _trafo( std::move( rhv._trafo ) ), _computed( std::move( rhv._computed ) ),
      _trafo_is( motor::move( rhv._trafo_is ) ), _computed_os( motor::move( rhv._computed_os ) )
{
}

//********************************************************************************
trafo3d_component::~trafo3d_component( void_t ) noexcept
{
    // icomponent should disconnect and release the slots.
    // so we do not do it here.
    // if( _trafo_is ) _trafo_is->disconnect();
    // if( _computed_os ) _computed_os->disconnect();
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

    _trafo_is = this_t::create_input_slot< motor::math::m3d::trafof_t >( "[trafo3d_component] : trafo_is" );
    _computed_os = this_t::create_output_slot< motor::math::m3d::trafof_t >( "[trafo3d_component] : trafo_os" );
}

//**********************************************************************************
void_t trafo3d_component::sync_inputs( void_t ) noexcept
{
    _trafo = _trafo_is->pull_data();
}

//**********************************************************************************
void_t trafo3d_component::sync_outputs( void_t ) noexcept {}