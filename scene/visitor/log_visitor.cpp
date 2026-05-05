#include "log_visitor.h"
#include "../node/group.h"
#include "../node/leaf.h"
#include "../node/decorator.h"
#include "../node/trafo3d_node.h"
#include "../node/camera_node.h"
#include "../component/name_component.hpp"

#include <motor/log/global.h>

using namespace motor::scene ;

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
motor::scene::result log_visitor::visit( motor::scene::decorator_ptr_t nptr ) noexcept 
{
    this_t::print( "> decorator" + this_t::get_name_component( nptr ) ) ;
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
motor::scene::result log_visitor::visit( motor::scene::group_ptr_t nptr )  noexcept
{
    this_t::print( "> group"+ this_t::get_name_component( nptr ) ) ;
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
motor::scene::result log_visitor::visit( motor::scene::leaf_ptr_t nptr ) noexcept
{
    this_t::print( "> leaf" + this_t::get_name_component( nptr ) ) ;
    return motor::scene::ok ;
}

//*********************************************************************
motor::scene::result log_visitor::visit( motor::scene::camera_node_ptr_t nptr ) noexcept 
{
    this_t::print( "> camera" + this_t::get_name_component( nptr ) ) ;
    return motor::scene::ok ;
}

//*********************************************************************
motor::scene::result log_visitor::visit( motor::scene::trafo3d_node_ptr_t nptr ) noexcept
{
    this_t::print( "> trafo" + this_t::get_name_component( nptr ) ) ;
    ++_indent ;
    return motor::scene::ok ;
}

//*********************************************************************
motor::scene::result log_visitor::post_visit( motor::scene::trafo3d_node_ptr_t, motor::scene::result const ) noexcept
{
    --_indent ;
    this_t::print( "< trafo") ;
    return motor::scene::ok ;
}

//*********************************************************************
motor::string_t log_visitor::get_name_component( motor::scene::node_cptr_t nptr ) const 
{
    auto * cptr = nptr->borrow_component<motor::scene::name_component>() ;
    if( cptr != nullptr )
    {
        return " (" + cptr->get_name() + ")" ;
    }
    return "" ;
}