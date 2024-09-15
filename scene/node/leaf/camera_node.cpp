
#include "camera_node.h"
#include "../../visitor/ivisitor.h"

#include <motor/log/global.h>

using namespace motor::scene ;

//*************************************************************************
camera_node::camera_node( void_t ) noexcept
{}

//*************************************************************************
camera_node::camera_node( motor::gfx::generic_camera_mtr_safe_t cam ) noexcept : 
    _cam( motor::move( cam ) )
{
}

//*************************************************************************
camera_node::camera_node( this_rref_t rhv ) noexcept : base_t( std::move(rhv) ), 
    _cam( motor::move( rhv._cam ) )
{}

//*************************************************************************
camera_node::~camera_node( void_t ) noexcept
{
    motor::release( motor::move( _cam ) ) ;
}

//*************************************************************************
motor::scene::result camera_node::apply( motor::scene::ivisitor_ptr_t vptr ) noexcept
{
    auto res = vptr->visit( this ) ;
    if( motor::scene::no_success( res ) )
    {
        motor::log::global_t::warning( "[vzt_scene::camera_node::apply] : visit" ) ;
    }
    
    return res ;
}

//*************************************************************************
