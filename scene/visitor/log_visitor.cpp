#include "log_visitor.h"
#include "../node/group.h"
#include "../node/leaf.h"
#include "../node/decorator.h"

#include <motor/log/global.h>

using namespace motor::scene ;

motor_core_dd_id_init( log_visitor ) ;

//*********************************************************************
log_visitor::~log_visitor( void_t ) noexcept
{
}

//*********************************************************************
motor::string_t log_visitor::indent( void_t ) const noexcept
{
    motor::string_t ret ;
    for ( size_t i = 0; i < _indent; ++i ) ret += "    " ;
    return ret ;
}

//*********************************************************************
void_t log_visitor::print( motor::string_in_t s ) const noexcept
{
    motor::log::global::status( this_t::indent() + s ) ;
}

//************************************************************************
motor::scene::result log_visitor::visit( motor::scene::ivisitable_ptr_t vptr ) noexcept 
{
    if( dynamic_cast< motor::scene::group_ptr_t>( vptr ) != nullptr )
    {
        return this->visit( static_cast< motor::scene::group_ptr_t>( vptr ) ) ;
    }
    else if ( dynamic_cast<motor::scene::decorator_ptr_t>( vptr ) != nullptr )
    {
        return this->visit( static_cast< motor::scene::decorator_ptr_t>( vptr ) ) ;
    }
    else if( dynamic_cast<motor::scene::leaf_ptr_t>( vptr ) != nullptr )
    {
        return this->visit( static_cast< motor::scene::leaf_ptr_t>( vptr ) ) ;
    }

    return motor::scene::result::not_implemented ;
}

//************************************************************************
motor::scene::result log_visitor::post_visit( motor::scene::ivisitable_ptr_t vptr, motor::scene::result const r ) noexcept 
{
    if ( dynamic_cast<motor::scene::group_ptr_t>( vptr ) != nullptr )
    {
        return this->post_visit( static_cast<motor::scene::group_ptr_t>( vptr ), r ) ;
    }
    else if ( dynamic_cast<motor::scene::decorator_ptr_t>( vptr ) != nullptr )
    {
        return this->post_visit( static_cast<motor::scene::decorator_ptr_t>( vptr ), r ) ;
    }

    return motor::scene::result::not_implemented ;
}

//*********************************************************************
motor::scene::result log_visitor::visit( motor::scene::decorator_ptr_t ) noexcept 
{
    this_t::print( "> decorator" ) ;
    ++_indent ;
    return motor::scene::ok ;
}

//*********************************************************************
motor::scene::result log_visitor::post_visit( motor::scene::decorator_ptr_t, motor::scene::result const ) noexcept 
{
    --_indent ;
    this_t::print( "< decorator" ) ;
    return motor::scene::ok ;
}

//*********************************************************************
motor::scene::result log_visitor::visit( motor::scene::group_ptr_t )  noexcept
{
    this_t::print( "> group" ) ;
    ++_indent ;
    return motor::scene::ok ;
}

//*********************************************************************
motor::scene::result log_visitor::post_visit( motor::scene::group_ptr_t, motor::scene::result const ) noexcept
{
    --_indent ;
    this_t::print( "< group" ) ;
    return motor::scene::ok ;
}

//*********************************************************************
motor::scene::result log_visitor::visit( motor::scene::leaf_ptr_t ) noexcept
{
    this_t::print( "> leaf" ) ;
    return motor::scene::ok ;
}

//*********************************************************************
motor::scene::result log_visitor::visit( motor::scene::camera_node_ptr_t ) noexcept 
{
    this_t::print( "> camera" ) ;
    return motor::scene::ok ;
}

//*********************************************************************
motor::scene::result log_visitor::visit( motor::scene::trafo3d_node_ptr_t ) noexcept
{
    this_t::print( "> trafo" ) ;
    ++_indent ;
    return motor::scene::ok ;
}

//*********************************************************************
motor::scene::result log_visitor::post_visit( motor::scene::trafo3d_node_ptr_t, motor::scene::result const ) noexcept
{
    --_indent ;
    this_t::print( "< trafo" ) ;
    return motor::scene::ok ;
}

//*********************************************************************