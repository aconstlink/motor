
#include "switch_group.h"

#include "../../visitor/ivisitor.h"

using namespace motor::scene ;

//*******************************************************************
switch_group::switch_group( void_t ) noexcept
{
}

//*******************************************************************
switch_group::switch_group( node_ptr_t ptr ) noexcept : base_t(ptr)
{
}

//*******************************************************************
switch_group::switch_group( this_rref_t rhv ) noexcept : base_t( std::move(rhv))
{
    _actives = std::move(rhv._actives) ;
}

//*******************************************************************
switch_group::~switch_group( void_t ) noexcept
{
}

//*******************************************************************
motor::scene::result switch_group::switch_child( size_t const child_i, bool_t const active ) noexcept
{
    if( child_i >= base_t::get_num_children() ) 
        return motor::scene::result::invalid_argument ;

    _actives[child_i] = active ;

    return motor::scene::result::ok ;
}

//*******************************************************************
motor::scene::result switch_group::switch_off_all( void_t ) noexcept
{
    for( size_t i=0; i<_actives.size(); ++i ) _actives[i] = false ;
    return motor::scene::result::ok ;
}

//*******************************************************************
motor::scene::result switch_group::switch_on( node_ptr_t nptr ) noexcept
{
    if( nptr == nullptr ) 
        return motor::scene::result::invalid_argument ;

    size_t const index = base_t::find_index( nptr ) ;
    if( index == size_t(-1) ) 
        return motor::scene::result::failed ;

    _actives[index] = true ;

    return motor::scene::result::ok ;
}

//*******************************************************************
motor::scene::result switch_group::add_child( node_mtr_safe_t nptr, bool_t const active ) noexcept
{
    if( nptr == nullptr ) 
        return motor::scene::result::invalid_argument ;

    if( motor::scene::no_success( base_t::add_child( nptr ) ) )
        return motor::scene::result::failed ;
        
    _actives.push_back( active ) ;

    return motor::scene::result::ok ;
}

//*******************************************************************
motor::scene::result switch_group::remove_child( node_ptr_t nptr ) noexcept
{
    if( nptr == nullptr ) 
        return motor::scene::result::invalid_argument ;

    size_t const index = base_t::remove_child( nptr ) ;
    if( index == size_t(-1) ) 
        return motor::scene::result::ok ;
    
    _actives.erase( _actives.begin() + index ) ;

    return motor::scene::result::ok ;
}

//*******************************************************************
motor::scene::result switch_group::apply( motor::scene::ivisitor_ptr_t vptr ) noexcept
{
    auto const r = vptr->visit( this ) ;
    this_t::traverse_children( vptr, [=]( size_t i ) { return this->_actives[i] ; } ) ;
    return vptr->post_visit( this, r ) ;
}

//*******************************************************************
motor::scene::result switch_group::detach( node_ptr_t which_ptr ) noexcept
{
    size_t index = base_t::remove_child( which_ptr ) ;
    if( index == size_t(-1) ) 
        return motor::scene::result::failed ;

    _actives.erase( _actives.begin() + index ) ;
    return motor::scene::result::ok ;
}
