
#include "logic_leaf.h"
#include "../../visitor/ivisitor.h"

#include <motor/log/global.h>

using namespace motor::scene ;

//*************************************************************************
logic_leaf::logic_leaf( void_t ) noexcept
{}

//*************************************************************************
logic_leaf::logic_leaf( node_ptr_t pptr ) noexcept : base_t(pptr)
{}

//*************************************************************************
logic_leaf::logic_leaf( this_rref_t rhv ) noexcept : base_t( std::move(rhv) )
{}

//*************************************************************************
logic_leaf::~logic_leaf( void_t ) noexcept
{}

//*************************************************************************
motor::scene::result logic_leaf::apply( motor::scene::ivisitor_ptr_t vptr ) noexcept
{
    auto res = vptr->visit( this ) ;
    if( motor::scene::no_success( res ) )
    {
        motor::log::global_t::warning( "[vzt_scene::logic_leaf::apply] : visit" ) ;
    }
    
    return res ;
}

//*************************************************************************
