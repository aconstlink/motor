
#include "global.h"
#include "system.h"

#include <motor/log/global.h>

using namespace motor::io ;

motor::concurrent::mutex_t global::_mtx ;
global * global::_ptr = nullptr ;

//***********************************************************************
global::global( void_t )
{
    _ios = motor::memory::global_t::alloc( motor::io::system_t(),
        "[io::global] : job_scheduler" ) ;
}

//***********************************************************************
global::global( this_rref_t rhv )
{
    motor_move_member_ptr( _ios, rhv ) ;
}

//***********************************************************************
global::~global( void_t )
{
    motor::memory::global_t::dealloc( _ios ) ;
}

//***********************************************************************
global::this_ptr_t global::init( void_t )
{
    if( _ptr != nullptr )
        return this_t::_ptr ;

    {
        motor::concurrent::lock_guard_t lk( this_t::_mtx ) ;

        if( _ptr != nullptr )
            return this_t::_ptr ;

        this_t::_ptr = motor::memory::global_t::alloc( this_t(),
            "[motor::io::global::init]" ) ;

        motor::log::global::status( "[online] : motor io" ) ;
    }

    return this_t::_ptr ;
}

//***********************************************************************
void_t global::deinit( void_t )
{
    if( _ptr == nullptr )
        return ;

    motor::memory::global_t::dealloc( _ptr ) ;
    _ptr = nullptr ;
}

//***********************************************************************
global::this_ptr_t global::get( void_t )
{
    return this_t::init() ;
}

//***********************************************************************
motor::io::system_ptr_t global::io_system( void_t )
{
    return this_t::get()->_ios ;
}

//***********************************************************************
motor::io::load_handle_t global::load( motor::io::path_cref_t file_path, motor::io::obfuscator_rref_t obf )
{
    return this_t::io_system()->load( file_path, std::move( obf ) ) ;
}

//***********************************************************************
motor::io::load_handle_t global::load( motor::io::path_cref_t file_path,
    size_t const offset, size_t const sib, motor::io::obfuscator_rref_t obf ) 
{
    return this_t::io_system()->load( file_path, offset, sib, std::move( obf ) ) ;
}

//***********************************************************************
motor::io::store_handle_t global::store( motor::io::path_cref_t file_path, char_cptr_t data, size_t const sib ) 
{
    return this_t::io_system()->store( file_path, data, sib ) ;
}

//***********************************************************************
motor::io::store_handle_t global::store( motor::io::path_cref_t file_path, char_cptr_t data, size_t const sib, motor::io::obfuscator_rref_t obf )
{
    return this_t::io_system()->store( file_path, data, sib, std::move( obf ) ) ;
}