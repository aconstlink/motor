
#include "decorator.h"

#include "../global.h"
#include "../visitor/ivisitor.h"

#include <motor/memory/global.h>

using namespace motor::scene ;

motor_core_dd_id_init( decorator ) ;

//*************************************************************************
decorator::decorator( void_t ) noexcept
{}

//*************************************************************************
decorator::decorator( this_rref_t rhv ) noexcept  : base_t( std::move(rhv) )
{
    _decorated = motor::move( rhv._decorated ) ;

    if(_decorated != nullptr )
        _decorated->set_parent(this) ;
}

//*************************************************************************
decorator::decorator( node_ptr_t decorated_ptr ) noexcept  : _decorated( decorated_ptr )
{}

//*************************************************************************
decorator::~decorator( void_t ) noexcept
{
   if( _decorated != nullptr )
   {
       _decorated->set_parent(nullptr) ;
       motor::memory::release_ptr( _decorated ) ;
   }
}

//*************************************************************************
motor::scene::result decorator::apply( motor::scene::ivisitor_ptr_t vptr ) noexcept
{
    //motor::scene::result const r = vptr->visit( this ) ;
    motor::scene::result const r = motor::scene::global::resolve( vptr, this ).visit( vptr, this ) ;

    if( motor::scene::success( r ) )
    {
        traverse_decorated( vptr ) ;
    }
    
    //return vptr->post_visit( this, r ) ;
    return motor::scene::global::resolve( vptr, this ).post_visit( vptr, this, r ) ;
}

//*************************************************************************
motor::scene::result decorator::replace( node_ptr_t which_ptr, node_ptr_t with_ptr ) noexcept
{
    if( which_ptr == with_ptr ) 
        return motor::scene::result::ok ;

    if( which_ptr == nullptr ) 
        return motor::scene::result::invalid_argument ;

    if( which_ptr != _decorated ) 
        return motor::scene::result::invalid_argument ;

    if( _decorated != nullptr ) 
        _decorated->set_parent( nullptr ) ;

    _decorated = with_ptr ;

    if( _decorated != nullptr ) 
        _decorated->set_parent( this ) ;

    return motor::scene::result::ok ;
}

//*************************************************************************
motor::scene::result decorator::detach( node_ptr_t which_ptr ) noexcept
{
    if( which_ptr == nullptr ) 
        return motor::scene::result::ok ;

    if( which_ptr != _decorated ) 
        return motor::scene::result::invalid_argument ;

    _decorated->set_parent( nullptr ) ;
    _decorated = nullptr ;

    return motor::scene::result::ok ;
}

//*************************************************************************
void_t decorator::set_decorated( node_mtr_safe_t nptr, bool_t const reset_parent ) noexcept
{
    if( _decorated != nullptr && reset_parent ) 
        _decorated->set_parent(nullptr) ;

    _decorated = nptr ;

    if( _decorated != nullptr ) 
        _decorated->set_parent(this) ;
}

//*************************************************************************
void_t decorator::set_decorated( node_mtr_safe_t nptr ) noexcept
{
    set_decorated( nptr, true ) ;
}

//*************************************************************************
node_ptr_t decorator::borrow_decorated( void_t ) noexcept
{
    return _decorated ;
}

//*************************************************************************
node_cptr_t decorator::borrow_decorated( void_t ) const noexcept
{
    return _decorated ;
}

//*************************************************************************
motor::scene::result decorator::traverse_decorated( motor::scene::ivisitor_ptr_t ptr ) noexcept
{
    motor::scene::result r = motor::scene::result::repeat ;
    while( r == motor::scene::result::repeat )
    {
        if( _decorated == nullptr ) break ;
        r = motor::scene::node_t::derived_apply( _decorated ).apply(ptr) ;
    }
    return r ;
}

//*************************************************************************
decorator::this_ref_t decorator::operator = ( this_rref_t rhv ) noexcept
{
    base_t::operator=( std::move(rhv) ) ;
    _decorated = motor::move( rhv._decorated ) ;
    return *this ;
}
