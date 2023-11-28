
#include "system.h"

#include <motor/concurrent/typedefs.h>

#include <motor/log/global.h>

#include <iostream>
#include <fstream>

using namespace motor::io ;

//************************************************************************************
system::system( void_t ) noexcept
{}

//************************************************************************************
system::~system( void_t ) noexcept
{
    while( _load_stack.has_item() )
    {
        this_t::load_item_ptr_t ptr = _load_stack.pop() ;
        motor::memory::release_ptr( ptr ) ;
    }

    while( _store_stack.has_item() )
    {
        this_t::store_item_ptr_t ptr = _store_stack.pop() ;
        motor::memory::release_ptr( ptr ) ;
    }
}

//************************************************************************************
system::system( this_rref_t rhv ) noexcept
{
    _load_stack = std::move( rhv._load_stack ) ;
    _store_stack = std::move( rhv._store_stack ) ;
}

//************************************************************************************
motor::io::system::load_handle_t system::load( motor::io::path_cref_t file_path, 
    std::launch const lt, motor::io::obfuscator_rref_t obf) noexcept
{
    return this_t::load( file_path, size_t( 0 ), size_t( -1 ), lt, std::move( obf ) );
}

//************************************************************************************
motor::io::system::load_handle_t system::load( motor::io::path_cref_t file_path,
    size_t const offset, size_t const sib, std::launch const lt, motor::io::obfuscator_rref_t obf  ) noexcept
{
    this_t::load_item_ptr_t li = this_t::get_load_item() ;

    li->ftr = std::async( lt, [=]( void_t )
    {
        std::ifstream is( file_path, std::ifstream::binary ) ;

        if( is.is_open() )
        {
            // process content
            {
                is.seekg( 0, is.end ) ;
                size_t const length = sib != size_t(-1) ? sib : size_t( is.tellg() ) ;
                is.seekg( offset, is.beg ) ;
                
                char_ptr_t data = motor::memory::global::alloc_raw<char_t>( length, 
                    "[io::system::load] : load data" ) ;

                is.read( data, length ) ;
                
                li->sib = is.gcount() ;
                li->data = data ;

                // li->data will be taken over and returned by res.move_ptr()
                auto res = motor::io::obfuscator_t( std::move(obf) ).decode( li->data, li->sib ) ;

                li->sib = res.sib() ;
                li->data = res.move_ptr() ;
            }

            return motor::io::result::ok ;
        }

        return motor::io::result::file_does_not_exist ;
    } ) ;

    return this_t::load_handle_t( li, this ) ;
}

//************************************************************************************
motor::io::system::store_handle_t system::store( motor::io::path_cref_t file_path, 
    char_cptr_t data_ptr, size_t const sib, std::launch const lt, motor::io::obfuscator_rref_t obf ) noexcept
{
    this_t::store_item_ptr_t si = this_t::get_store_item() ;

    si->ftr = std::async( lt, [=]( void_t )
    {
        motor::filesystem::path p( file_path ) ;

        // 1. check if the directory exists. if not, create it
        if( !motor::filesystem::exists( p.parent_path() ) )
        {
            auto const ec = motor::filesystem::create_directories( p.parent_path() ) ;

            if( !ec )
            {
                motor::log::global::error( "[motor::io::system::store] : create directories : " ) ;
                return motor::io::result::failed ;
            }
        }

        std::ofstream os( file_path, std::ifstream::binary ) ;

        if( os.is_open() )
        {
            auto res = motor::io::obfuscator_t( std::move(obf) ).encode( data_ptr, sib ) ;
            os.write( res.ptr(), res.sib() ) ;

            return motor::io::result::ok ;
        }
        return motor::io::result::file_does_not_exist ;
    } ) ;

    return this_t::store_handle_t( si, this ) ;
}

//************************************************************************************
motor::io::result system::wait_for_operation( this_t::load_handle_rref_t hnd, 
    motor::io::load_completion_funk_t funk ) noexcept
{
    this_t::load_item_ptr_t li = this_t::load_item_ptr_t(hnd._dptr) ;
    if( li == nullptr ) return motor::io::result::invalid_argument ;
    
    // taking over the handle, so release the internal pointer
    motor::memory::release_ptr( motor::move(hnd._dptr) ) ;

    // 1. wait for the operation
    auto const res = li->ftr.get() ;

    // 2. call the user funk
    {
        funk( li->data, li->sib, res ) ;
    }

    // 3. reclaim the item
    {
        motor::memory::global::dealloc_raw( li->data ) ;
        *li = this_t::load_item_t() ;
    }
    
    // 4. store item for later use
    {
        motor::concurrent::lock_guard_t lk( _load_mtx ) ;
        _load_stack.push( li ) ;
    }

    return res ;
}

//************************************************************************************
motor::io::result system::wait_for_operation( this_t::store_handle_rref_t hnd, 
    motor::io::store_completion_funk_t funk ) noexcept
{
    this_t::store_item_ptr_t si = this_t::store_item_ptr_t( hnd._dptr ) ;
    if( si == nullptr ) return motor::io::result::invalid_argument ;

    // taking over the handle, so release the internal pointer
    motor::memory::release_ptr( motor::move(hnd._dptr) ) ;

    // 1. wait for the operation
    auto const res = si->ftr.get() ;

    // 2. call the user funk
    {
        funk( res ) ;
    }

    // 3. reclaim the item
    {
        *si = this_t::store_item_t() ;
    }

    // 4. store item for later use
    {
        motor::concurrent::lock_guard_t lk( _store_mtx ) ;
        _store_stack.push( si ) ;
    }

    return res ;
}

//************************************************************************************
system::load_item_ptr_t system::get_load_item( void_t ) noexcept
{
    motor::concurrent::lock_guard_t lk( _load_mtx ) ;
    return _load_stack.has_item() ? _load_stack.pop() : 
        motor::memory::create_ptr( load_item_t(), "[system::get_load_item] : item" ).mtr() ;
}

//************************************************************************************
system::store_item_ptr_t system::get_store_item( void_t ) noexcept
{
    return _store_stack.has_item() ? _store_stack.pop() :
        motor::memory::create_ptr( store_item_t(), "[system::get_store_item] : item" ).mtr() ;
}