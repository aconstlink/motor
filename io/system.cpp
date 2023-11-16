
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
        motor::memory::global::dealloc( ptr ) ;
    }

    while( _store_stack.has_item() )
    {
        this_t::store_item_ptr_t ptr = _store_stack.pop() ;
        motor::memory::global::dealloc( ptr ) ;
    }
}

//************************************************************************************
system::system( this_rref_t rhv ) noexcept
{
    _load_stack = ::std::move( rhv._load_stack ) ;
    _store_stack = ::std::move( rhv._store_stack ) ;
}

//************************************************************************************
motor::io::load_handle_t system::load( motor::io::path_cref_t file_path, motor::io::obfuscator_rref_t obf) noexcept
{
    return this_t::load( file_path, size_t( 0 ), size_t( -1 ), std::move( obf ) );
}

//************************************************************************************
motor::io::load_handle_t system::load( motor::io::path_cref_t file_path,
    size_t const offset, size_t const sib, motor::io::obfuscator_rref_t obf  ) noexcept
{
    this_t::load_item_ptr_t li = this_t::get_load_item() ;

    motor::concurrent::thread_t( [=]( void_t )
    {
        std::ifstream is( file_path, std::ifstream::binary ) ;

        if( is.is_open() )
        {
            // process content
            {
                is.seekg( 0, is.end ) ;
                size_t const length = sib != size_t(-1) ? sib : size_t( is.tellg() ) ;
                is.seekg( offset, is.beg ) ;
                
                char_ptr_t data = motor::memory::global::alloc_raw<char_t>( length ) ;

                is.read( data, length ) ;
                
                li->sib = is.gcount() ;
                li->data = data ;

                auto res = motor::io::obfuscator_t( std::move(obf) ).decode( li->data, li->sib ) ;

                li->sib = res.sib() ;
                li->data = res.move_ptr() ;
            }

            // signal shared data
            {
                motor::concurrent::lock_guard_t lk( li->mtx ) ;
                li->ready = true ;
                li->status = motor::io::result::ok ;
            }
            li->cv.notify_all() ;
        }
        else
        {
            // signal shared data
            {
                motor::concurrent::lock_guard_t lk( li->mtx ) ;
                li->ready = true ;
                li->status = motor::io::result::file_does_not_exist ;
            }
            li->cv.notify_all() ;
        }
    } ).detach() ;

    return motor::io::load_handle_t( motor::io::internal_item_ptr_t(li), this ) ;
}

//************************************************************************************
motor::io::store_handle_t system::store( motor::io::path_cref_t file_path, 
    char_cptr_t data_ptr, size_t const sib, motor::io::obfuscator_rref_t obf ) noexcept
{
    this_t::store_item_ptr_t si = this_t::get_store_item() ;

    motor::concurrent::thread_t( [=]( void_t )
    {
        motor::filesystem::path p( file_path ) ;

        // 1. check if the directory exists. if not, create it
        if( !motor::filesystem::exists( p.parent_path() ) )
        {
            auto const ec = motor::filesystem::create_directories( p.parent_path() ) ;

            if( !ec )
            {
                motor::log::global::error( "[motor::io::system::store] : create directories : " ) ;

                // signal shared data
                {
                    motor::concurrent::lock_guard_t lk( si->mtx ) ;
                    si->ready = true ;
                    si->status = motor::io::result::failed ;
                }
                si->cv.notify_all() ;
                return ;
            }
        }

        std::ofstream os( file_path, std::ifstream::binary ) ;

        if( os.is_open() )
        {
            auto res = motor::io::obfuscator_t( std::move(obf) ).encode( data_ptr, sib ) ;
            os.write( res.ptr(), res.sib() ) ;

            // signal shared data
            {
                motor::concurrent::lock_guard_t lk( si->mtx ) ;
                si->ready= true ;
                si->status = motor::io::result::ok ;
            }
            si->cv.notify_all() ;
        }
        else
        {
            // signal shared data
            {
                motor::concurrent::lock_guard_t lk( si->mtx ) ;
                si->ready = true ;
                si->status = motor::io::result::file_does_not_exist ;
            }
            si->cv.notify_all() ;
        }
    } ).detach() ;

    return motor::io::store_handle_t( motor::io::internal_item_ptr_t( si ), this ) ;
}

//************************************************************************************
motor::io::result system::wait_for_operation( motor::io::load_handle_rref_t hnd, 
    motor::io::load_completion_funk_t funk ) noexcept
{
    this_t::load_item_ptr_t li = this_t::load_item_ptr_t(hnd._data_ptr) ;
    if( li == nullptr ) return motor::io::result::invalid_argument ;
    
    hnd._data_ptr = nullptr ;

    // 1. wait for the operation
    {
        motor::concurrent::lock_t lk( li->mtx ) ;
        while( !li->ready ) li->cv.wait( lk ) ;
    }

    auto const res = li->status ;

    // 2. call the user funk
    {
        funk( li->data, li->sib, li->status ) ;
    }

    // 3. reclaim the item
    {
        motor::memory::global::dealloc( li->data ) ;
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
motor::io::result system::wait_for_operation( motor::io::store_handle_rref_t hnd, 
    motor::io::store_completion_funk_t funk ) noexcept
{
    this_t::store_item_ptr_t si = this_t::store_item_ptr_t( hnd._data_ptr ) ;
    if( si == nullptr ) return motor::io::result::invalid_argument ;

    hnd._data_ptr = nullptr ;

    // 1. wait for the operation
    {
        motor::concurrent::lock_t lk( si->mtx ) ;
        while( !si->ready ) si->cv.wait( lk ) ;
    }

    auto const res = si->status ;

    // 2. call the user funk
    {
        funk( si->status ) ;
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
        motor::memory::global::alloc( load_item_t(), "[system::get_load_item] : item" ) ;
}

//************************************************************************************
system::store_item_ptr_t system::get_store_item( void_t ) noexcept
{
    return _store_stack.has_item() ? _store_stack.pop() :
        motor::memory::global::alloc( store_item_t(), "[system::get_store_item] : item" ) ;
}