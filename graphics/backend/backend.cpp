

#include "backend.h"

#include <motor/log/global.h>

using namespace motor::graphics ;

//****
size_t backend::create_backend_id( void_t ) noexcept
{
    static std::mutex mtx ;
    static size_t id = 0 ;

    assert( id <= motor::graphics::max_backends ) ;

    std::lock_guard< std::mutex > lk( mtx ) ;
    return id++ ;
} 

//****
backend::backend( void_t ) noexcept
{
    //_backend_id = this_t::create_backend_id() ;
}

//****
backend::backend( motor::graphics::backend_type const bt ) : _bt( bt )
{
    //_backend_id = this_t::create_backend_id() ;
}

//****
backend::backend( this_rref_t rhv ) noexcept
{
    //_backend_id = rhv._backend_id ;
    //rhv._backend_id = size_t( -1 ) ;
    _bt = rhv._bt ;
    rhv._bt = backend_type::unknown ;
}

//****
backend::~backend( void_t ) 
{
    motor::log::global_t::status( this_t::get_bid() != size_t( -1 ),
         "destruction of backend type [" + motor::graphics::to_string(_bt) + "] with id : " + motor::to_string( this_t::get_bid() ) ) ;
}