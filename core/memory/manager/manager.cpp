
#include "manager.h"

#include <motor/core/log/global.h>
#include <iostream>
#include <cstdlib>

using namespace motor::memory ;

//*************************************************************************************
manager::manager( void_t ) noexcept 
{

}

//*************************************************************************************
manager::~manager( void_t ) noexcept
{

}

//*************************************************************************************
manager::this_ptr_t manager::create( void_t ) noexcept
{
    return new this_t() ;
}

//*************************************************************************************
void_t manager::destroy( this_ptr_t ptr ) noexcept
{
    delete ptr ;
}

//*************************************************************************************
void_ptr_t manager::create( size_t const sib, motor::memory::purpose_cref_t purpose ) noexcept 
{
    return this_t::alloc( sib, purpose, true ) ;
}

//*************************************************************************************
void_ptr_t manager::create( size_t const sib ) noexcept 
{
    return this_t::create( sib, "[manager::create] : no purpose speficied" ) ;
}

//*************************************************************************************
void_ptr_t manager::create( void_ptr_t ptr ) noexcept 
{
    lock_t lk( _mtx ) ;

    auto iter = _ptr_to_info.find( ptr ) ;

    motor::log::global_t::error_and_exit( iter == _ptr_to_info.end(), 
                "[manager::create] : ptr is not found in manager." ) ;

    motor::log::global_t::error_and_exit( iter->second.rc == size_t(-1), 
                "[manager::create] : non-managed pointer is not ref counted." ) ;

    ++iter->second.rc ;

    return ptr ;
}

//*************************************************************************************
void_ptr_t manager::release( void_ptr_t ptr ) noexcept 
{
    return this_t::dealloc( ptr, true ) ;
}

//*************************************************************************************
void_ptr_t manager::alloc( size_t const sib, motor::memory::purpose_cref_t purpose, bool_t const managed ) noexcept 
{
    if( sib == 0 ) return nullptr ;

    void_ptr_t ptr = malloc( sib ) ;
    {
        lock_t lk( _mtx ) ;
        _ptr_to_info[ptr] = memory_info{sib, managed ? size_t(1) : size_t(-1), purpose} ;
        _allocated_sib += sib ;
    }
    return ptr ;
}

//*************************************************************************************
void_ptr_t manager::alloc( size_t const sib, motor::memory::purpose_cref_t purpose ) noexcept 
{
    return this_t::alloc( sib, purpose, false ) ;
}

//*************************************************************************************
void_ptr_t manager::alloc( size_t const sib ) noexcept 
{
    return this_t::alloc( sib, "[manager::alloc]" ) ;
}

//*************************************************************************************
void_ptr_t manager::dealloc( void_ptr_t ptr, bool_t const managed ) noexcept 
{
    if( ptr == nullptr ) return nullptr ;

    {
        lock_t lk(_mtx) ;
        auto iter = _ptr_to_info.find( ptr ) ;

        motor::log::global_t::error_and_exit( iter == _ptr_to_info.end(), 
                "[manager::dealloc] : ptr location not found in manager." ) ;

        if( managed )
        {
            motor::log::global_t::error_and_exit( iter->second.rc == size_t(-1), 
                "[manager::release] : non-managed pointer can not be released." ) ;

            if( --(iter->second.rc) != size_t(0) )
            {
                return ptr ;
            }
        }
        
        _allocated_sib -= iter->second.sib ;
        _ptr_to_info.erase( iter ) ;
    }
    free( ptr ) ;
    return nullptr ;
}

//*************************************************************************************
void manager::dealloc( void_ptr_t ptr ) noexcept 
{
    this_t::dealloc( ptr, false ) ;
}

//*************************************************************************************
size_t manager::get_sib( void_t ) const noexcept
{
    return _allocated_sib ;
}

//*************************************************************************************
bool_t manager::get_purpose( void_ptr_t ptr, motor::memory::purpose_ref_t pout ) const noexcept 
{
    if( ptr == nullptr ) return false ;

    {
        lock_t lk( _mtx ) ;
        auto const iter = _ptr_to_info.find( ptr ) ;

        if( motor::log::global::error( iter == _ptr_to_info.end(),
            "[manager::get_purpose] : ptr location not found" ) )
            return false ;

        pout = iter->second.purpose ;        
    }

    return true ;
}

//*************************************************************************************
void_t manager::dump_to_std( void_t ) const noexcept 
{
    lock_t lk(_mtx) ;

    std::cout << "***************************************************" << std::endl ;
    std::cout << "[manager::dump_to_std] : Dump to std [" << _allocated_sib << "] sib" << std::endl ;

    for( auto iter : _ptr_to_info )
    {
        std::cout << iter.second.purpose << "; sib: " << std::to_string(iter.second.sib) << 
            "; rc: " << std::to_string(iter.second.rc) << std::endl ;
    }

    if( _ptr_to_info.empty() )
    {
        std::cout << "* Memory manager has no entries." << std::endl ;
    }
}

//*************************************************************************************
void_t manager::destroy( void_t ) noexcept
{
    this_t::destroy( this ) ;
}