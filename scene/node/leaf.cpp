
#include "leaf.h"
#include "../visitor/ivisitor.h"

#include <motor/log/global.h>

using namespace motor::scene ;

motor_core_dd_id_init( leaf ) ;

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
    return vptr->visit( this ) ;
}
