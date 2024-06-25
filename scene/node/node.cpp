
#include "node.h"
#include "../component/icomponent.h"
#include "../visitor/ivisitor.h"

using namespace motor::scene ;

//*******************************************************************
node::traverser::traverser( node_ptr_t begin ) noexcept :_traverse( begin )
{
}

//*******************************************************************
void_t node::traverser::apply( motor::scene::ivisitor_ptr_t v ) noexcept
{
    v->on_start() ;
    _traverse->apply( v ) ;
    v->on_finish() ;
}

//*******************************************************************
node::derived_apply::derived_apply( node_ptr_t begin ) noexcept :_traverse( begin )
{
}

//*******************************************************************
motor::scene::result node::derived_apply::apply( motor::scene::ivisitor_ptr_t v ) noexcept
{
    return _traverse->apply( v ) ;
}

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
    _components = std::move( rhv._components ) ;
}

//*******************************************************************
node::this_ref_t node::operator = ( this_rref_t rhv ) noexcept
{
    _parent = motor::move( rhv._parent ) ;
    _components = std::move( rhv._components ) ;
    return *this ;
}

//*******************************************************************
node::~node( void_t ) noexcept
{
    for( auto * comp : _components ) 
    {
        motor::memory::release_ptr( comp ) ;
    }
}

//*******************************************************************
motor::scene::result node::apply( motor::scene::ivisitor_ptr_t v ) noexcept
{
    return v->visit( this ) ;
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