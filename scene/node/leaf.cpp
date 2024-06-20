
#include "leaf.h"
#include "../visitor/ivisitor.h"

#include <motor/log/global.h>

using namespace motor::scene ;

//*******************************************************************
leaf::leaf( void_t ) noexcept
{
}

//*******************************************************************
leaf::leaf( motor::scene::node_ptr_t ptr ) noexcept : base_t(ptr)
{
}

//*******************************************************************
leaf::leaf( this_rref_t rhv ) noexcept : base_t( std::move(rhv) )
{
}

//*******************************************************************
leaf::~leaf( void_t )  noexcept
{
}

//*******************************************************************
motor::scene::result leaf::apply( motor::scene::ivisitor_ptr_t vptr ) noexcept
{
    auto res = vptr->visit( this ) ;
    if( motor::scene::no_success(res) )
    {
        motor::log::global::error( "[vzt_scene::leaf::apply] : visit" ) ;
    }
    return vptr->post_visit( this ) ;
}
