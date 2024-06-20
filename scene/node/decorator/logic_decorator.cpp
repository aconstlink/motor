
#include "logic_decorator.h"
#include "../../visitor/ivisitor.h"

#include <motor/log/global.h>

using namespace motor::scene ;

//*************************************************************************
logic_decorator::logic_decorator( void_t ) noexcept
{}

//*************************************************************************
logic_decorator::logic_decorator( this_rref_t rhv ) noexcept : base_t( std::move(rhv) )
{}

//*************************************************************************
logic_decorator::logic_decorator( node_ptr_t pptr ) noexcept : base_t(pptr)
{}

//*************************************************************************
logic_decorator::~logic_decorator( void_t ) noexcept
{}

//*************************************************************************
void_t logic_decorator::set_decorated( node_mtr_safe_t nptr ) noexcept
{
    base_t::set_decorated( nptr ) ;
}

//*************************************************************************
motor::scene::result logic_decorator::apply( motor::scene::ivisitor_ptr_t vptr ) noexcept
{
    auto res = vptr->visit( this ) ;
    if( motor::scene::no_success( res )  )
    {
        motor::log::global_t::error( "[vzt_scene::logic_decorator::apply] : visit" ) ;
    }

    base_t::traverse_decorated( vptr ) ;
    return vptr->post_visit( this ) ;
}
