
#include "config_graphics_component.h"

using namespace motor::scene;

//*************************************************************
config_graphics_component::config_graphics_component( void_t ) noexcept {}

//*************************************************************
config_graphics_component::config_graphics_component( this_rref_t rhv ) noexcept
    : _msl( motor::move( rhv._msl ) ),
      _geo( motor::move( rhv._geo ) ), _imgs( std::move( rhv._imgs ) )
{
    _msl_status = motor::shared( motor::graphics::command_status_t() ) ;
    _geo_status = motor::shared( motor::graphics::command_status_t() ) ;
}

//*************************************************************
config_graphics_component::~config_graphics_component( void_t ) noexcept
{
    for( auto & img : _imgs )
    {
        motor::release( motor::move( img.img ) );
        motor::release( motor::move( img.status ) );
    }

    motor::release( motor::move( _msl ) );
    motor::release( motor::move( _msl_status ) );
    motor::release( motor::move( _geo ) );
    motor::release( motor::move( _geo_status ) );
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
bool_t config_graphics_component::init_and_cleanup( motor::graphics::gen4::frontend_ptr_t fe ) noexcept
{
    // init msl
    {
        auto const s = fe->decode( *_msl_status ) ;
        bool_t const a = s == motor::graphics::command_status::status::configured ;
        bool_t const b = s == motor::graphics::command_status::status::in_transit ;

        if( !a && !b )
        {
            fe->configure< motor::graphics::msl_object_t >( _msl, _msl_status );
        }
    }

    // init geo
    {
        auto const s = fe->decode( *_geo_status ) ;
        bool_t const a = s == motor::graphics::command_status::status::configured ;
        bool_t const b = s == motor::graphics::command_status::status::in_transit ;

        if( !a && !b )
        {
            fe->configure< motor::graphics::geometry_object_t >( _geo, _geo_status );
        }
    }

    return true;
}

//*************************************************************