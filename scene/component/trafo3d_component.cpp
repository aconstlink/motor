
#include "trafo3d_component.h"

using namespace motor::scene;

//********************************************************************************
trafo3d_component::trafo3d_component( void_t ) noexcept
{
    this_t::init_slots();
}

//********************************************************************************
trafo3d_component::trafo3d_component( motor::math::m3d::trafof_cref_t trafo ) noexcept
    : _trafo( trafo )
{
    this_t::init_slots();
    _trafo_is->set_value( trafo );
}

//********************************************************************************
trafo3d_component::trafo3d_component( this_rref_t rhv ) noexcept
    : icomponent( std::move( rhv ) ), _trafo( std::move( rhv._trafo ) ),
      _computed( std::move( rhv._computed ) ), _trafo_is( motor::move( rhv._trafo_is ) ),
      _computed_os( motor::move( rhv._computed_os ) ), _composer( std::move( rhv._composer ) )
{
}

//********************************************************************************
trafo3d_component::~trafo3d_component( void_t ) noexcept
{
    // icomponent should disconnect and release the slots.
    // so we do not do it here.
    // if( _trafo_is ) _trafo_is->disconnect();
    // if( _computed_os ) _computed_os->disconnect();

    for( auto * ptr : _composer )
    {
        // the owner needs to disconnect, otherwise
        // there will be some dangling node pointers.
        ptr->disconnect();
        motor::release( motor::move( ptr ) );
    }
}

//********************************************************************************
trafo3d_component::trafo_composer_mtr_safe_t trafo3d_component::create_composer( void_t ) noexcept
{
    auto ret = motor::shared( this_t::trafo_composer_t() );
    _composer.emplace_back( motor::share( ret ) );
    return motor::move( ret );
}

//********************************************************************************
trafo3d_component::trafo_composer_mtr_t
trafo3d_component::create_composer_and_borrow( void_t ) noexcept
{
    auto ret = motor::shared( this_t::trafo_composer_t() );
    _composer.emplace_back( motor::move( ret ) );
    return _composer.back() ;
}

//********************************************************************************
bool_t trafo3d_component::attach_composer( trafo_composer_mtr_safe_t mtr ) noexcept
{
    auto iter = std::find( _composer.begin(), _composer.end(), mtr.mtr() );
    if( iter != _composer.end() )
    {
        motor::release( motor::move( mtr ) );
        return false;
    }

    _composer.emplace_back( motor::move( mtr ) );
    return true;
}

//********************************************************************************
bool_t trafo3d_component::has_composer( trafo_composer_mtr_t ptr ) const noexcept
{
    auto iter = std::find( _composer.begin(), _composer.end(), ptr );
    return iter != _composer.end() ? true : false;
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
    _trafo = _trafo_is->pull_data();
}

//**********************************************************************************
void_t trafo3d_component::sync_outputs( void_t ) noexcept {}