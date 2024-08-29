#include "log_visitor.h"
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

//*********************************************************************
motor::scene::result log_visitor::visit( motor::scene::node_ptr_t ) noexcept
{
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
