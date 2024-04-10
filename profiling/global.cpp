

#include "global.h"


motor::profiling::manager_t motor::profiling::global::_mgr = motor::profiling::manager_t() ;

using namespace motor::profiling ;

//******************************************************
global::global( void_t ) noexcept
{

}

//******************************************************
global::global( this_rref_t rhv ) noexcept
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