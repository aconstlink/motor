

#include "global.h"

using namespace motor::profiling ;

//******************************************************
global::global( void_t ) noexcept
{

}

//******************************************************
global::global( this_rref_t /*rhv*/ ) noexcept
{
}

//******************************************************
global::~global( void_t ) noexcept
{

}

#if MOTOR_PROFILING
//******************************************************
void_t global::push( motor::string_cref_t name ) noexcept
{
    this_t::manager().push( name ) ;
}

//******************************************************
void_t global::pop( void_t ) noexcept
{
    this_t::manager().pop() ;
}

#endif

//******************************************************
void_t global::deinit( void_t ) noexcept
{
    _mgr.~manager() ;
}