
#include "global.h"

#include "manager/manager.h"

using namespace motor::memory ;

//*************************************************************************************
global::global( void_t ) noexcept {}

//*************************************************************************************
global::global( this_rref_t rhv ) noexcept 
{
    _manager = std::move( rhv._manager ) ;
}

//*************************************************************************************
global::~global( void_t ) noexcept 
{}

//*************************************************************************************
void global::dump_to_std( void_t ) noexcept 
{
    this_t::_manager.dump_to_std() ;
}

//*************************************************************************************
void_ptr_t global::create( size_t const sib, char_cptr_t purpose ) noexcept 
{
    return this_t::_manager.create( sib, purpose ) ;
}

//*************************************************************************************
void_ptr_t global::create( size_t const sib ) noexcept 
{
    return this_t::_manager.create( sib ) ;
}

//*************************************************************************************
void_ptr_t global::create( void_ptr_t ptr ) noexcept 
{
    return this_t::_manager.create( ptr ) ;
}

//*************************************************************************************
void_ptr_t global::release( void_ptr_t ptr, motor::memory::void_funk_t funk ) noexcept 
{
    return this_t::_manager.release( ptr, funk ) ;
}

//*************************************************************************************
void_ptr_t global::alloc( size_t const sib, char_cptr_t purpose ) noexcept 
{
    return this_t::_manager.alloc( sib, purpose ) ;
}

//*************************************************************************************
void_ptr_t global::alloc( size_t const sib ) noexcept 
{
    return this_t::_manager.alloc( sib ) ;
}

//*************************************************************************************
void_ptr_t global::dealloc( void_ptr_t ptr ) noexcept 
{
    if( ptr == nullptr ) return nullptr ;

    this_t::_manager.dealloc( ptr ) ;

    return nullptr ;
}

//*************************************************************************************
size_t global::get_sib( void_t ) noexcept 
{
    return this_t::_manager.get_sib() ;
}

//*************************************************************************************
bool_t global::get_purpose( void_ptr_t ptr, char_cptr_t & pout ) noexcept 
{
    return this_t::_manager.get_purpose( ptr, pout ) ;
}
