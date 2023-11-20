
#include "global.h"
#include "system.h"

#include <motor/log/global.h>

using namespace motor::io ;

motor::concurrent::mutex_t global::_mtx ;
global * global::_ptr = nullptr ;

//***********************************************************************
global::global( void_t ) noexcept
{
    _ios = motor::memory::global_t::alloc( motor::io::system_t(),
        "[io::global] : job_scheduler" ) ;
}

//***********************************************************************
global::global( this_rref_t rhv ) noexcept
{
    motor_move_member_ptr( _ios, rhv ) ;
}

//***********************************************************************
global::~global( void_t ) noexcept
{
    motor::memory::global_t::dealloc( _ios ) ;
}

//***********************************************************************
global::this_ptr_t global::init( void_t ) noexcept
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
void_t global::deinit( void_t ) noexcept
{
    if( _ptr == nullptr )
        return ;

    motor::memory::global_t::dealloc( _ptr ) ;
    _ptr = nullptr ;
}

//***********************************************************************
global::this_ptr_t global::get( void_t ) noexcept
{
    return this_t::init() ;
}

//***********************************************************************
motor::io::system_ptr_t global::io_system( void_t ) noexcept
{
    return this_t::get()->_ios ;
}

//***********************************************************************
motor::io::system_t::load_handle_t global::load( motor::io::path_cref_t file_path, 
    std::launch const lt, motor::io::obfuscator_rref_t obf ) noexcept
{
    return this_t::io_system()->load( file_path, lt, std::move( obf ) ) ;
}

//***********************************************************************
motor::io::system_t::load_handle_t global::load( motor::io::path_cref_t file_path,
    size_t const offset, size_t const sib, std::launch const lt, motor::io::obfuscator_rref_t obf )  noexcept
{
    return this_t::io_system()->load( file_path, offset, sib, lt, std::move( obf ) ) ;
}

//***********************************************************************
motor::io::system_t::store_handle_t global::store( motor::io::path_cref_t file_path, char_cptr_t data, 
    size_t const sib, std::launch const lt ) noexcept
{
    return this_t::io_system()->store( file_path, data, sib, lt ) ;
}

//***********************************************************************
motor::io::system_t::store_handle_t global::store( motor::io::path_cref_t file_path, char_cptr_t data, size_t const sib, 
    motor::io::obfuscator_rref_t obf, std::launch const lt ) noexcept
{
    return this_t::io_system()->store( file_path, data, sib, lt, std::move( obf ) ) ;
}