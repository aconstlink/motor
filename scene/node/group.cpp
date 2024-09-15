
#include "group.h"

#include "../visitor/ivisitor.h"

#include <motor/log/global.h>
#include <motor/memory/global.h>

#include <algorithm>

using namespace motor::scene ;

//*******************************************************************
group::group( void_t ) noexcept
{}

//*******************************************************************
group::group( node_ptr_t parent_ptr ) noexcept  : node(parent_ptr)
{}

//*******************************************************************
group::group( this_rref_t rhv ) noexcept : base_t(std::move(rhv))
{
    _children = std::move( rhv._children ) ;
    for( auto nptr : _children )
        nptr->set_parent( this ) ;
}

//*******************************************************************
group::~group( void_t ) noexcept
{
    for( auto * c : _children )
    { 
        c->set_parent(nullptr) ; 
        motor::memory::release_ptr( c ) ;
    }
}

//*******************************************************************
motor::scene::result group::apply( motor::scene::ivisitor_ptr_t vptr ) noexcept
{
    motor::scene::result const r = vptr->visit( this ) ;

    switch( r ) 
    {
    case motor::scene::ok:
        traverse_children( vptr, _funk ) ;
        break ;
    default:
        break ;
    }
    
    return vptr->post_visit( this, r ) ;
}

//*******************************************************************
motor::scene::result group::replace( node_ptr_t which_ptr, node_ptr_t with_ptr ) noexcept
{
    if( with_ptr == nullptr ) 
        return motor::scene::invalid_argument ;

    if( which_ptr == with_ptr ) 
        return motor::scene::ok ;

    auto found = std::find( _children.begin(), _children.end(), which_ptr ) ;
    if( found == _children.end() ) 
        return motor::scene::invalid_argument ;

    which_ptr->set_parent( nullptr ) ;
    with_ptr->set_parent( this ) ;

    size_t const n = found - _children.begin() ;
    _children[n] = with_ptr ;

    return motor::scene::ok ;
}

//*******************************************************************
motor::scene::result group::detach( node_ptr_t which_ptr ) noexcept
{
    if( which_ptr == nullptr ) 
        return motor::scene::ok ;

    auto found = std::find( _children.begin(), _children.end(), which_ptr ) ;
    if( found == _children.end() ) 
        return motor::scene::invalid_argument ;

    which_ptr->set_parent( nullptr ) ;
    _children.erase( found ) ;

    return motor::scene::ok ;
}

//*******************************************************************
size_t group::get_num_children( void_t ) const noexcept
{
    return _children.size() ;
}

//*******************************************************************
node_ptr_t group::borrow_child( size_t const child_i )  noexcept
{
    if( child_i >= _children.size() ) 
        return nullptr ;

    return _children[child_i] ;
}

//*******************************************************************
size_t group::find_index( node_ptr_t nptr ) const noexcept
{
    if( nptr == nullptr )
        return size_t(-1) ;

    auto found = std::find( _children.begin(), _children.end(), nptr ) ;
    size_t const index = found - _children.begin() ;
    
    return index < _children.size() ? index : size_t(-1) ;
}

//*******************************************************************
motor::scene::result group::add_child( node_mtr_safe_t nptr ) noexcept
{
    if( nptr == nullptr || nptr == this ) 
        return motor::scene::invalid_argument ;

    auto found = std::find( _children.begin(), _children.end(), nptr.mtr() ) ;
    if( found != _children.end() ) 
        return motor::scene::invalid_argument ;

    _children.push_back(nptr) ;
    nptr->set_parent(this) ;

    return motor::scene::ok ;
}

//*******************************************************************
size_t group::remove_child( node_ptr_t nptr ) noexcept
{
    auto found = std::find( _children.begin(), _children.end(), nptr ) ;
    if( found == _children.end() ) 
        return size_t(-1) ;
    
    size_t const index = found - _children.begin() ;
    
    (*found)->set_parent(nullptr) ;
    _children.erase(found) ;
    
    return index ;
}

//*******************************************************************
void_t group::traverse_children( motor::scene::ivisitor_ptr_t ptr ) noexcept
{
    this_t::traverse_children( ptr, _funk ) ;
}

//*******************************************************************
void_t group::traverse_children( motor::scene::ivisitor_ptr_t ptr, 
    traverse_predicate_t func ) noexcept
{
    for( size_t i=0; i<_children.size(); ++i )
    {
        if( !func( i ) ) continue ;
        
        motor::scene::result r = motor::scene::repeat ;
        while( r == motor::scene::repeat )
        {
            auto * nptr = _children[i] ;
            r = motor::scene::node_t::derived_apply(nptr).apply( ptr ) ;
        }
    }
}