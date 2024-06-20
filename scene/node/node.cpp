
#include "node.h"


using namespace motor::scene ;

//*******************************************************************
node::node( void_t ) noexcept
{}

//*******************************************************************
node::node( node::this_ptr_t parent_ptr ) noexcept  : _parent(parent_ptr)
{}

//*******************************************************************
node::node( this_rref_t rhv ) noexcept
{
    _parent = motor::move( rhv._parent ) ;
}

//*******************************************************************
node::this_ref_t node::operator = ( this_rref_t rhv ) noexcept
{
    _parent = motor::move( rhv._parent ) ;
    return *this ;
}

//*******************************************************************
node::~node( void_t ) noexcept
{}

//*******************************************************************
motor::scene::result node::apply( motor::scene::ivisitor_ptr_t ) noexcept
{
    return motor::scene::result::ok ;
}

//*******************************************************************
node::this_ptr_t node::borrow_parent( void_t ) noexcept
{
    return _parent ;
}

//*******************************************************************
node::this_ptr_t node::set_parent( node::this_ptr_t parent_ptr ) noexcept
{
    auto old_parent = _parent ;
    _parent = parent_ptr ;
    return old_parent ;
}