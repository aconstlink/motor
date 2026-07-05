
#include "config_graphics_component.h"

using namespace motor::scene;

//*************************************************************
config_graphics_component::config_graphics_component( void_t ) noexcept {}

//*************************************************************
config_graphics_component::config_graphics_component( this_rref_t rhv ) noexcept
    : _stats( std::move( rhv._stats ) ), _msl( motor::move( rhv._msl ) ), _geo( motor::move( rhv._geo ) ),
      _imgs( std::move( rhv._imgs ) )
{
}

//*************************************************************
config_graphics_component::~config_graphics_component( void_t ) noexcept
{
    for( auto iter : _stats )
    {
        assert( iter.second != this_t::status::in_transit );
    }

    for( auto * img : _imgs )
    {
        motor::release( motor::move( img ) );
    }

    motor::release( motor::move( _msl ) );
    motor::release( motor::move( _geo ) );
}

//*************************************************************
void_t config_graphics_component::set_msl( motor::graphics::msl_object_safe_t obj ) noexcept
{
    _msl = motor::move( obj );
}

//*************************************************************
void_t config_graphics_component::set_geo( motor::graphics::geometry_object_safe_t obj ) noexcept
{
    _geo = motor::move( obj );
}

//*************************************************************
void_t config_graphics_component::add_img( motor::graphics::image_object_safe_t obj ) noexcept
{
    _imgs.emplace_back( motor::move( obj ) );
}

//*************************************************************
bool_t config_graphics_component::init_and_cleanup( size_t const wid,
                                                    motor::graphics::gen4::frontend_ptr_t fe ) noexcept
{
    auto iter = _stats.find( wid ) ;
    if( iter == _stats.end() )
    {
        _stats[wid] = this_t::status::uninitilized ;
        iter = _stats.find( wid ) ;
    }

    if( iter->second == this_t::status::released ) return true;
    if( iter->second == this_t::status::ready )
    {
        #if 0
        // cleanup
        motor::release( motor::move( _msl ) );
        motor::release( motor::move( _geo ) );
        for( auto * img : _imgs )
        {
            motor::release( motor::move( img ) );
        }
        iter->second = this_t::status::released;
        #endif
        return true;
    }

    this_t::do_init( wid, fe );

    return false;
}

//*************************************************************
config_graphics_component::status
config_graphics_component::do_init( size_t const wid, motor::graphics::gen4::frontend_ptr_t fe ) noexcept
{
    // this should not happen. Handle status before.
    auto iter = _stats.find( wid ) ;
    if( iter == _stats.end() )
    {
        return this_t::status::uninitilized ;
    }

    if( iter->second != this_t::status::uninitilized ) return iter->second;
    iter->second = this_t::status::in_transit;

    for( auto * img : _imgs )
    {
        fe->configure< motor::graphics::image_object_t >( img );
    }

    if( _geo != nullptr )
    {
        fe->configure< motor::graphics::geometry_object_t >( _geo );
    }

    if( _msl != nullptr )
    {
        fe->configure< motor::graphics::msl_object_t >( _msl );
    }

    fe->fence( [ &, wid ]( void_t ) 
    { 
    this->_stats[wid] = this_t::status::ready; 
    } );

    return iter->second;
}

//*************************************************************