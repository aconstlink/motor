

#include "global.h"

using namespace motor::device ;

motor::device::system_ptr_t global::_sys = nullptr ;


//****************************************************************************
motor::device::system_ptr_t global::system( void_t ) 
{
    return this_t::init() ;
}

//****************************************************************************
motor::device::system_ptr_t global::init( void_t ) noexcept 
{
    if( _sys == nullptr ) 
    {
        _sys = motor::memory::global_t::alloc< 
            motor::device::system_t >( "Global Device System" )  ;
    }

    return _sys ;
}

//****************************************************************************
void_t global::deinit( void_t ) noexcept 
{
    motor::memory::global_t::dealloc( _sys ) ;
}