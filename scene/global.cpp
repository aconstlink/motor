

#include "global.h"

#include "node/group.h"
#include "node/decorator.h"
#include "node/trafo3d_node.h"
#include "node/render_node.h"
#include "node/render_settings.h"
#include "visitor/trafo_visitor.h"
#include "visitor/log_visitor.h"
#include "visitor/render_visitor.h"

using namespace motor::scene ;

motor::scene::double_dispatcher_t global::_dd = motor::scene::double_dispatcher_t() ;

//***********************************************************************
motor::scene::double_dispatch_callbacks global::resolve( motor::scene::ivisitor_ptr_t a, motor::scene::ivisitable_ptr_t b ) noexcept 
{
    return _dd.resolve( a, b ) ;
}

//***********************************************************************
void_t global::init( void_t ) noexcept 
{
    this_t::register_default_callbacks< motor::scene::trafo_visitor, motor::scene::trafo3d_node >() ;
    //this_t::register_default_callbacks< motor::scene::trafo_visitor, motor::scene::trafo3d_node >() ;

    this_t::register_default_callbacks< motor::scene::log_visitor, motor::scene::group >() ;
    this_t::register_default_callbacks< motor::scene::log_visitor, motor::scene::decorator >() ;

    //this_t::register_default_callbacks< motor::scene::render_visitor, motor::scene::group >() ;
    //this_t::register_default_callbacks< motor::scene::render_visitor, motor::scene::decorator >() ;
    this_t::register_default_callbacks< motor::scene::render_visitor, motor::scene::trafo3d_node >() ;
    this_t::register_default_callbacks< motor::scene::render_visitor, motor::scene::render_settings >() ;
    this_t::register_default_callbacks< motor::scene::render_visitor, motor::scene::render_node >() ;
}

//***********************************************************************
void_t global::deinit( void_t ) noexcept 
{
    _dd.~double_dispatcher() ;
}