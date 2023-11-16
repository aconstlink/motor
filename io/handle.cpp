
#include "handle.h"
#include "system.h"

#include <motor/core/macros/move.h>
#include <cassert>

using namespace motor::io ;

//************************************************************************************
load_handle::load_handle( motor::io::internal_item_ptr_t ptr, motor::io::system_ptr_t ios_ptr )
{
    _data_ptr = ptr ;
    _ios = ios_ptr ;
}

//************************************************************************************
load_handle::load_handle( void_t )
{}

//************************************************************************************
load_handle::load_handle( this_rref_t rhv )
{
    motor_move_member_ptr( _data_ptr, rhv ) ;
    motor_move_member_ptr( _ios, rhv ) ;
}

//************************************************************************************
load_handle::~load_handle( void_t )
{
    assert( _data_ptr == nullptr ) ;
}

//************************************************************************************
load_handle::this_ref_t load_handle::operator = ( this_rref_t rhv )
{
    motor_move_member_ptr( _data_ptr, rhv ) ;
    motor_move_member_ptr( _ios, rhv ) ;
    return *this ;
}

//************************************************************************************
motor::io::result load_handle::wait_for_operation( motor::io::load_completion_funk_t funk )
{
    if( _ios == nullptr ) 
        return motor::io::result::invalid_handle ;

    auto * ios = _ios ;
    return ios->wait_for_operation( this_t( ::std::move( *this )), funk ) ;
}

bool_t load_handle::can_wait( void_t ) const noexcept { return _data_ptr != nullptr ; }

//************************************************************************************
store_handle::store_handle( motor::io::internal_item_ptr_t ptr, motor::io::system_ptr_t ios_ptr  )
{
    _data_ptr = ptr ;
    _ios = ios_ptr ;
}

//************************************************************************************
store_handle::store_handle( void_t )
{}

//************************************************************************************
store_handle::store_handle( this_rref_t rhv )
{
    motor_move_member_ptr( _data_ptr, rhv ) ;
    motor_move_member_ptr( _ios, rhv ) ;
}

//************************************************************************************
store_handle::~store_handle( void_t )
{
    assert( _data_ptr == nullptr ) ;
}

//************************************************************************************
store_handle::this_ref_t store_handle::operator = ( this_rref_t rhv )
{
    motor_move_member_ptr( _data_ptr, rhv ) ;
    motor_move_member_ptr( _ios, rhv ) ;
    return *this ;
}

//************************************************************************************
motor::io::result store_handle::wait_for_operation( void_t )
{
    if( _ios == nullptr )
        return motor::io::result::invalid_handle ;

    auto * ios = _ios ;
    return ios->wait_for_operation( this_t( ::std::move( *this ) ), [=] ( motor::io::result ) { } ) ;
}

//************************************************************************************
motor::io::result store_handle::wait_for_operation( motor::io::store_completion_funk_t funk )
{
    if( _ios == nullptr )
        return motor::io::result::invalid ;

    auto * ios = _ios ;
    return ios->wait_for_operation( this_t( ::std::move( *this ) ), funk ) ;
}

//************************************************************************************