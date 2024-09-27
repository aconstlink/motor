
#include "logic_group.h"

using namespace motor::scene ;

//*******************************************************************
logic_group::logic_group( void_t ) noexcept
{}

//*******************************************************************
logic_group::logic_group( node_ptr_t ptr ) noexcept : base_t(ptr)
{}

//*******************************************************************
logic_group::logic_group( this_rref_t rhv ) noexcept : base_t(std::move(rhv))
{}

//*******************************************************************
logic_group::~logic_group( void_t ) noexcept
{}

//*******************************************************************
motor::scene::result logic_group::add_child( node_mtr_safe_t nptr ) noexcept
{
    return base_t::add_child( nptr ) ;
}

//*******************************************************************
size_t logic_group::remove_child( node_ptr_t nptr ) noexcept
{
    return base_t::remove_child( nptr ) ;
}