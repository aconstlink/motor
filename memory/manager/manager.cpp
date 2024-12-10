
#include "manager.h"

#include "../arena.hpp"

#include <iostream>
#include <cstdlib>
#include <cassert>
#include <iostream>
#include <string>
#include <atomic>

using namespace motor::memory ;

namespace this_file
{
    struct sib_16
    {
        char _dummy[16] ;
    };

    struct sib_32
    {
        char _dummy[ 32 ] ;
    };
}

struct motor::memory::manager::pimpl
{
    using area_16_t = motor::memory::arena< this_file::sib_16, motor::memory::detail::arena_allocator > ;
    area_16_t a16 = area_16_t( 1000 ) ;

    using area_32_t = motor::memory::arena< this_file::sib_32, motor::memory::detail::arena_allocator > ;
    area_32_t a32 = area_32_t( 1000 ) ;

    pimpl( void_t ) noexcept{}
    ~pimpl( void_t ) noexcept 
    {
        pimpl::deinit() ;
    }

    void_t init( void_t ) noexcept
    {
    }

    void_t deinit( void_t ) noexcept
    {
        a16.~arena() ;
        a32.~arena() ;
    }
};

//*************************************************************************************
manager::manager( void_t ) noexcept 
{
    #if MOTOR_MEMORY_OBSERVER
    _observer = new ( malloc( sizeof( motor::memory::observer_t ) ) ) motor::memory::observer_t ;
    #endif

    _pimpl = new ( malloc( sizeof( this_t::pimpl ) ) ) this_t::pimpl ;
    _pimpl->init() ;
}

//*************************************************************************************
manager::manager( this_rref_t rhv ) noexcept : _allocated_sib( rhv._allocated_sib ),
    _ptr_to_info( std::move( rhv._ptr_to_info ))
{
    rhv._allocated_sib = 0 ;

    // @note potential leak. Old observer will be lost.
    // depends of where the observer is coming from.
    _observer = motor::move( rhv._observer ) ;

    if( _pimpl != nullptr ) 
    {
        _pimpl->deinit() ;
        delete( _pimpl ) ;
    }
    _pimpl = motor::move( rhv._pimpl ) ;
}

//*************************************************************************************
manager::~manager( void_t ) noexcept
{
#if MOTOR_MEMORY_OBSERVER
    free( _observer ) ;
#endif

    if( _pimpl != nullptr ) 
        delete( _pimpl ) ;
}

//*************************************************************************************
manager::this_ref_t manager::operator = ( this_rref_t rhv ) noexcept 
{
    _allocated_sib = rhv._allocated_sib ;
    rhv._allocated_sib = 0 ;
    _ptr_to_info = std::move( rhv._ptr_to_info ) ;
    _observer = motor::move( rhv._observer ) ;

    if( _pimpl != nullptr ) delete( _pimpl ) ;
    _pimpl = motor::move( rhv._pimpl ) ;

    return *this ;
}

//*************************************************************************************
void_ptr_t manager::create( size_t const sib, char_cptr_t purpose ) noexcept 
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
    if( ptr == nullptr ) return nullptr ;

    lock_t lk( _mtx ) ;

    auto iter = _ptr_to_info.find( ptr ) ;

    assert( iter != _ptr_to_info.end() && "[manager::create] : ptr is not found in manager." ) ;
    assert( iter->second.rc != size_t(-1) && "[manager::create] : non-managed pointer is not ref counted." ) ;

    ++iter->second.rc ;

    #if MOTOR_MEMORY_OBSERVER
    _observer->on_ref_inc() ;
    #endif

    return ptr ;
}

#define USE_SIB_OPTIMIZATION 1

//*************************************************************************************
void_ptr_t manager::alloc( size_t const sib, char_cptr_t purpose, bool_t const managed ) noexcept 
{
    if( sib == 0 ) return nullptr ;

    void_ptr_t ptr = nullptr ;
    
    #if USE_SIB_OPTIMIZATION
    if( sib <= 16 )
    {
        ptr = reinterpret_cast<void_ptr_t>( _pimpl->a16.alloc() ) ;
    }    
    else if( sib <= 32 ) 
    {
        ptr = reinterpret_cast<void_ptr_t>( _pimpl->a32.alloc() ) ;
    }
    else
    #endif
    {
        ptr = malloc( sib ) ;
    }
        

    {
        lock_t lk( _mtx ) ;
        _ptr_to_info[ptr] = memory_info{sib, managed ? size_t(1) : size_t(-1), purpose} ;
        _allocated_sib += sib ;
    }

#if MOTOR_MEMORY_OBSERVER
    if ( _observer->on_alloc( sib, managed ) > 500000 )
    {
        _observer->swap_and_clear() ;
        std::cout << "[MOTOR_MEMORY_OBSERVER] : swap and clear" << std::endl ;
    }
#endif

    return ptr ;
}

//*************************************************************************************
void_ptr_t manager::alloc( size_t const sib, char_cptr_t purpose ) noexcept 
{
    return this_t::alloc( sib, purpose, false ) ;
}

//*************************************************************************************
void_ptr_t manager::alloc( size_t const sib ) noexcept 
{
    return this_t::alloc( sib, "[manager::alloc]" ) ;
}

//*************************************************************************************
void_ptr_t manager::release( void_ptr_t ptr, motor::memory::void_funk_t rel_funk ) noexcept 
{
    if( ptr == nullptr ) return nullptr ;
    
    size_t sib = 0 ;

    {
        lock_t lk(_mtx) ;
        auto iter = _ptr_to_info.find( ptr ) ;

        assert( iter != _ptr_to_info.end() && "[manager::release] : ptr is not found in manager." ) ;

        assert( iter->second.rc != size_t(-1) && "[manager::release] : non-managed pointer is not ref counted." ) ;

        if( --(iter->second.rc) != size_t(0) )
        {
            #if MOTOR_MEMORY_OBSERVER
            _observer->on_ref_dec() ;
            #endif

            return ptr ;
        }

        sib = iter->second.sib ;

        _allocated_sib -= iter->second.sib ;
        _ptr_to_info.erase( iter ) ;
    }

    rel_funk() ;

    #if USE_SIB_OPTIMIZATION
    if( sib <= 16 )
        _pimpl->a16.dealloc( reinterpret_cast<this_file::sib_16*>(ptr) ) ;
    else if( sib <= 32 )
        _pimpl->a32.dealloc( reinterpret_cast<this_file::sib_32*>(ptr) ) ;
    else 
    #endif
    {
        free( ptr ) ;
    }

    #if MOTOR_MEMORY_OBSERVER
    _observer->on_dealloc( sib ) ;
    #endif

    return nullptr ;
}

//*************************************************************************************
void manager::dealloc( void_ptr_t ptr ) noexcept 
{
    if( ptr == nullptr ) return ;

    size_t sib = 0 ;

    {
        lock_t lk(_mtx) ;
        auto iter = _ptr_to_info.find( ptr ) ;

        assert( iter != _ptr_to_info.end() && "[manager::dealloc] : ptr is not found in manager." ) ;
        assert( iter->second.rc == size_t(-1) && "[manager::release] : managed pointer must be released" ) ;

        sib = iter->second.sib ;

        _allocated_sib -= iter->second.sib ;
        _ptr_to_info.erase( iter ) ;
    }

    #if USE_SIB_OPTIMIZATION
    if ( sib <= 16 )
        _pimpl->a16.dealloc( reinterpret_cast<this_file::sib_16 *>( ptr ) ) ;
    else if ( sib <= 32 )
        _pimpl->a32.dealloc( reinterpret_cast<this_file::sib_32 *>( ptr ) ) ;
    else
    #endif
    {free( ptr ) ;}

    #if MOTOR_MEMORY_OBSERVER
    _observer->on_dealloc( sib ) ;
    #endif
}

//*************************************************************************************
size_t manager::get_sib( void_t ) const noexcept
{
    return _allocated_sib ;
}

//*************************************************************************************
bool_t manager::get_purpose( void_ptr_t ptr, char_cptr_t & pout ) const noexcept 
{
    if( ptr == nullptr ) return false ;

    {
        lock_t lk( _mtx ) ;
        auto const iter = _ptr_to_info.find( ptr ) ;

        if( iter == _ptr_to_info.end() ) return false ;
        //if( motor::log::global::error( iter == _ptr_to_info.end(),
          //  "[manager::get_purpose] : ptr location not found" ) )
            return false ;

        pout = iter->second.purpose ;
    }

    return true ;
}

#if MOTOR_MEMORY_OBSERVER
//*************************************************************************************
motor::memory::observer_ptr_t manager::get_observer( void_t ) noexcept
{
    return _observer ;
}
#endif

//*************************************************************************************
void_t manager::dump_to_std( void_t ) const noexcept 
{
    lock_t lk(_mtx) ;

    std::cout << "***************************************************" << std::endl ;
    std::cout << "[manager::dump_to_std] : Dump to std [" << _allocated_sib << "] sib" << std::endl ;

    size_t sib_less_100 = 0 ;
    size_t num_entries_less_100 = 0 ;

    for( auto iter : _ptr_to_info )
    {
        if( iter.second.sib < 100 )
        {
            sib_less_100 += iter.second.sib ;
            ++num_entries_less_100 ;
            continue ;
        }

        std::string p(iter.second.purpose!= nullptr ? iter.second.purpose : "[manager::dump_to_std] : Unspecified") ;
        std::cout << p << 
            "; sib: " << std::to_string(iter.second.sib) << 
            "; rc: " << (iter.second.rc != size_t(-1) ? std::to_string(iter.second.rc) : "-") << std::endl ;
    }

    if( num_entries_less_100 > 0 )
    {
        std::cout << "[manager::dump_to_std] : " << std::to_string(num_entries_less_100) << 
            " # of entries with sib < 100 in total sib [" << std::to_string(sib_less_100) << "]" << std::endl ;
    }

    if( _ptr_to_info.empty() )
    {
        std::cout << "* Memory manager has no entries." << std::endl ;
    }
}