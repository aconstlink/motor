// SPDX-FileCopyrightText: 2023-2026 Alex Link
// SPDX-License-Identifier: MIT

#pragma once

#include "../typedefs.h"
#include <motor/concurrent/mrsw.hpp>

#include <atomic>

#define USE_DATAS_LOCKS 0

namespace motor
{
namespace platform
{
//*******************************************************************************************
template < typename T >
struct datas
{
    motor_this_typedefs( datas< T > );

  private:

#if USE_DATAS_LOCKS
    struct guard_data
    {
        std::thread::id tid;

        std::mutex mtx;
        std::condition_variable cv;

        size_t entries = 0;

        guard_data( void_t ) noexcept = default;
        guard_data( guard_data const & ) noexcept = delete;
        guard_data( guard_data && rhv ) noexcept
        {
            tid = std::move( rhv.tid );
            entries = rhv.entries;
        }
        guard_data & operator=( guard_data && rhv ) noexcept
        {
            assert( entries == 0 );
            tid = std::move( rhv.tid );
            entries = rhv.entries;
            return *this;
        }

        bool_t try_enter( void_t ) noexcept
        {
            std::unique_lock< std::mutex > lk( mtx );
            if( tid == std::this_thread::get_id() && ++entries > 0 ) return true;

            if( entries > 0 ) return false;

            tid = std::this_thread::get_id();
            ++entries;
            return true;
        }

        void_t enter( void_t ) noexcept
        {
            std::unique_lock< std::mutex > lk( mtx );
            if( tid == std::this_thread::get_id() && ++entries > 0 ) return;

            while( entries ) cv.wait( lk );

            tid = std::this_thread::get_id();
            ++entries;
        }

        bool_t leave( void ) noexcept
        {
            std::unique_lock< std::mutex > lk( mtx );
            if( tid == std::this_thread::get_id() && --entries == 0 )
            {
                tid = std::thread::id();
                cv.notify_one();
                return true;
            }
            return false;
        }
    };
#endif

  public:

#if USE_DATAS_LOCKS
    class try_guard
    {
      private:

        guard_data & _data;
        bool_t _accessed = false;

      public:

        try_guard( guard_data & d ) noexcept : _data( d )
        {
            _accessed = _data.try_enter();
        }

        ~try_guard( void_t ) noexcept
        {
            if( _accessed ) _data.leave();
        }

        bool_t has_accessed( void_t ) noexcept
        {
            return _accessed;
        }
    };

    class busy_guard
    {
      private:

        guard_data & _data;

      public:

        busy_guard( guard_data & d ) noexcept : _data( d )
        {
            _data.enter();
        }

        ~busy_guard( void_t ) noexcept
        {
            _data.leave();
        }
    };
#endif

  private:

    struct wrapper
    {
#if USE_DATAS_LOCKS
        // allow only one entity to access
        // the critical section.
        guard_data gd;
#endif

        bool_t valid = false;
        motor::string_t name;

        T * data = nullptr;

        wrapper( void_t ) noexcept {}
        wrapper( wrapper const & ) = delete;

        wrapper( wrapper && rhv ) noexcept
            :
#if USE_DATAS_LOCKS
              gd( std::move( rhv.gd ) ),
#endif
              valid( rhv.valid ), name( std::move( rhv.name ) ), data( motor::move( rhv.data ) )
        {
        }

        wrapper & operator=( wrapper && rhv ) noexcept
        {
#if USE_DATAS_LOCKS
            assert( gd.entries == 0 );

            gd = std::move( rhv.gd );
#endif
            valid = rhv.valid;
            rhv.valid = false;
            name = std::move( rhv.name );
            data = motor::move( rhv.data );
            return *this;
        }
    };

    using items_t = motor::vector< wrapper >;

#if USE_DATAS_LOCKS
    mutable motor::concurrent::mrsw_t mtx;
#endif
    items_t items;

  public:

    datas( void_t ) noexcept {}
    datas( this_cref_t ) = delete;
    datas( this_rref_t rhv ) noexcept : items( std::move( rhv.items ) ) {}
    this_ref_t operator=( this_cref_t ) = delete;
    this_ref_t operator=( this_rref_t rhv ) noexcept
    {
        items = std::move( rhv.items );
        return *this;
    }
    ~datas( void_t ) noexcept
    {
        this_t::invalidate_and_clear();
    }

  public: // read and write access

    // access if you are not sure if the object exists.
    // this function will also create that object
    bool_t access( size_t & oid, motor::string_in_t name,
        std::function< bool_t( size_t const, motor::string_in_t, T & ) > funk ) noexcept
    {
        // #1 quick check if id is still valid
        {
#if USE_DATAS_LOCKS
            motor::concurrent::mrsw_t::reader_lock_t lk( mtx );
#endif
            oid = this_t::check_oid( oid, items );
        }

        // #2 if new or invalid, create new oid
        if( oid == size_t( -1 ) )
        {
#if USE_DATAS_LOCKS
            motor::concurrent::mrsw_t::writer_lock_t lk( mtx );
#endif
            oid = this_t::determine_oid( oid, name, items );
        }

        // #3 oid is safe, can access now.
        if( oid != size_t( -1 ) )
        {
#if USE_DATAS_LOCKS
            motor::concurrent::mrsw_t::reader_lock_t lk( mtx );

            this_t::busy_guard busy_wait( items[ oid ].gd );
#endif
            return funk( oid, items[ oid ].name, *items[ oid ].data );
        }

        return false;
    }

    // access if you are not sure if the object exists.
    // this function will also create that object
    bool_t access( size_t & oid, motor::string_in_t name,
        std::function< bool_t( size_t const, T & ) > funk ) noexcept
    {
        return this_t::access( oid, name,
            [ & ]( size_t const, motor::string_in_t name_, T & d ) { return funk( oid, d ); } );
    }

    // access if you are not sure if the object exists.
    // this function will also create that object
    bool_t access( size_t & oid, motor::string_in_t name,
        std::function< bool_t( motor::string_in_t, T & ) > funk ) noexcept
    {
        return this_t::access( oid, name,
            [ & ]( size_t const, motor::string_in_t name_, T & d ) { return funk( name_, d ); } );
    }

    bool_t access(
        size_t & oid, motor::string_in_t name, std::function< bool_t( T & ) > funk ) noexcept
    {
        return this_t::access(
            oid, name, [ & ]( size_t const, motor::string_in_t, T & d ) { return funk( d ); } );
    }

  private: // internal access

    std::pair< bool_t, bool_t > try_access(
        size_t oid, std::function< void_t( this_t::wrapper & ) > funk ) noexcept
    {
        if( oid == size_t( -1 ) ) return std::make_pair( false, false );

        {
#if USE_DATAS_LOCKS
            motor::concurrent::mrsw_t::reader_lock_t lk( mtx );
#endif
            if( this_t::check_oid( oid, items ) == size_t( -1 ) )
                return std::make_pair( false, false );

#if USE_DATAS_LOCKS
            this_t::try_guard try_lock( items[ oid ].gd );
            if( !try_lock.has_accessed() ) return std::make_pair( true, false );
#endif
            funk( items[ oid ] );
        }
        return std::make_pair( false, true );
    }

  public: // try access

#if USE_DATAS_LOCKS
    std::pair< bool_t, bool_t > try_access(
        size_t oid, std::function< void_t( T & ) > funk ) noexcept
    {
        return this_t::try_access( oid, [ & ]( this_t::wrapper & wrp ) { funk( *wrp.data ); } );
    }

    std::pair< bool_t, bool_t > try_access(
        size_t oid, std::function< void_t( motor::string_in_t, T & ) > funk ) noexcept
    {
        return this_t::try_access(
            oid, [ & ]( this_t::wrapper & wrp ) { funk( wrp.name, *wrp.data ); } );
    }

    // access by id only
    // if you have an id and you know the object exists, use this function
    template < typename R >
    std::pair< bool_t, R > try_access( size_t const oid, std::function< R( T & ) > funk ) noexcept
    {
        if( oid == size_t( -1 ) ) return std::make_pair( false, R() );

        motor::concurrent::mrsw_t::reader_lock_t lk( mtx );

        if( this_t::check_oid( oid, items ) == size_t( -1 ) ) return std::make_pair( false, R() );

        this_t::try_guard try_lock( items[ oid ].gd );
        if( !try_lock.has_accessed() ) return std::make_pair( true, R() );

        return std::make_pair( true, funk( *items[ oid ].data ) );
    }
#else

    std::pair< bool_t, bool_t > try_access(
        size_t oid, std::function< void_t( T & ) > funk ) noexcept
    {
        return this_t::try_access( oid, [ & ]( this_t::wrapper & wrp ) { funk( *wrp.data ); } );
    }

    std::pair< bool_t, bool_t > try_access(
        size_t oid, std::function< void_t( motor::string_in_t, T & ) > funk ) noexcept
    {
        return this_t::try_access(
            oid, [ & ]( this_t::wrapper & wrp ) { funk( wrp.name, *wrp.data ); } );
    }

    // access by id only
    // if you have an id and you know the object exists, use this function
    template < typename R >
    std::pair< bool_t, R > try_access( size_t const oid, std::function< R( T & ) > funk ) noexcept
    {
        if( oid == size_t( -1 ) ) return std::make_pair( false, R() );

#if USE_DATAS_LOCKS
        motor::concurrent::mrsw_t::reader_lock_t lk( mtx );
#endif

        if( this_t::check_oid( oid, items ) == size_t( -1 ) ) return std::make_pair( false, R() );

        #if USE_DATAS_LOCKS
        this_t::try_guard try_lock( items[ oid ].gd );
        if( !try_lock.has_accessed() ) return std::make_pair( true, R() );
        #endif
        return std::make_pair( true, funk( *items[ oid ].data ) );
    }
#endif

  public: // read access

    bool_t access( size_t oid, std::function< void_t( T & ) > funk ) noexcept
    {
        return this_t::access( oid, [ = ]( motor::string_in_t, T & d ) { return funk( d ); } );
    }

    bool_t access(
        size_t oid, std::function< void_t( motor::string_in_t name, T & ) > funk ) noexcept
    {
        if( oid == size_t( -1 ) ) return false;

        {
#if USE_DATAS_LOCKS
            motor::concurrent::mrsw_t::reader_lock_t lk( mtx );
#endif
            if( this_t::check_oid( oid, items ) == size_t( -1 ) ) return false;

#if USE_DATAS_LOCKS
            this_t::busy_guard busy_wait( items[ oid ].gd );
#endif
            funk( items[ oid ].name, *items[ oid ].data );
        }
        return true;
    }

    bool_t access_by_name(
        motor::string_in_t name, std::function< void_t( size_t const, T & ) > funk ) noexcept
    {
#if USE_DATAS_LOCKS
        motor::concurrent::mrsw_t::reader_lock_t lk( mtx );
#endif
        auto i = size_t( -1 );
        while( ++i < items.size() && items[ i ].name != name );

        if( i == items.size() ) return false;

#if USE_DATAS_LOCKS
        this_t::busy_guard busy_wait( items[ i ].gd );
#endif
        funk( i, *items[ i ].data );

        return true;
    }

    // access by id only
    // if you have an id and you know the object exists, use this function
    template < typename R >
    std::pair< bool_t, R > access(
        size_t const oid, std::function< R( motor::string_in_t, T & ) > funk ) noexcept
    {
        if( oid == size_t( -1 ) ) return std::make_pair( false, R() );

#if USE_DATAS_LOCKS
        motor::concurrent::mrsw_t::reader_lock_t lk( mtx );
#endif
        if( this_t::check_oid( oid, items ) == size_t( -1 ) ) return std::make_pair( false, R() );

#if USE_DATAS_LOCKS
        this_t::busy_guard busy_wait( items[ oid ].gd );
#endif
        return std::make_pair( true, funk( items[ oid ].name, *items[ oid ].data ) );
    }

    // access by id only
    // if you have an id and you know the object exists, use this function
    template < typename R >
    std::pair< bool_t, R > access( size_t const oid, std::function< R( T & ) > funk ) noexcept
    {
        return this_t::access< R >( oid, [ = ]( motor::string_in_t, T & d ) { return funk( d ); } );
    }

  private:

    static size_t check_oid( size_t const oid, this_t::items_t & v ) noexcept
    {
        return oid < v.size() ? oid : size_t( -1 );
    }

    static size_t determine_oid(
        size_t oid, motor::string_cref_t name, this_t::items_t & v ) noexcept
    {
        if( this_t::check_oid( oid, v ) != size_t( -1 ) ) return oid;
        if( name.empty() ) return oid;

        assert( oid == size_t( -1 ) && "must be -1. if oid != -1 => object MUST be valid!" );

        // we have to search for the name first. This can happen
        // if a shader/msl object is recompiled.
        {
            while( ++oid < v.size() && v[ oid ].name != name );
            if( oid < v.size() ) return oid;
        }

        // look for the next invalid item to be reused.
        {
            oid = size_t( -1 );
            while( ++oid < v.size() && v[ oid ].valid );
        }

        if( oid >= v.size() && v.size() == v.capacity() )
        {
            oid = v.size();
            v.reserve( oid + 20 );
        }

        if( oid == v.size() )
        {
            this_t::wrapper w;
            w.valid = true;
            w.name = name;
            w.data = motor::memory::global::alloc< T >( typeid( T ).name() );

            v.emplace_back( std::move( w ) );
        }
        else
        {
            v[ oid ].valid = true;
            v[ oid ].name = name;
        }

        return oid;
    }

  public:

    size_t find_by_name( motor::string_in_t name ) const noexcept
    {
        if( name.empty() ) return size_t( -1 );

#if USE_DATAS_LOCKS
        motor::concurrent::mrsw_t::reader_lock_t lk( mtx );
#endif
        size_t i = size_t( -1 );
        while( ++i < items.size() && items[ i ].name != name );
        return i == items.size() ? size_t( -1 ) : i;
    }

    void_t for_each( std::function< void_t( T & ) > funk ) noexcept
    {
#if USE_DATAS_LOCKS
        motor::concurrent::mrsw_t::reader_lock_t lk( mtx );
#endif
        for( auto & i : items )
        {
            if( !i.valid ) continue;

#if USE_DATAS_LOCKS
            this_t::busy_guard busy_wait( i.gd );
#endif
            funk( *i.data );
        }
    }

    void_t for_each( std::function< void_t( size_t const, T & ) > funk ) noexcept
    {
#if USE_DATAS_LOCKS
        motor::concurrent::mrsw_t::reader_lock_t lk( mtx );
#endif
        for( size_t i = 0; i < items.size(); ++i )
        {
            if( !items[ i ].valid ) continue;

#if USE_DATAS_LOCKS
            this_t::busy_guard busy_wait( items[ i ].gd );
#endif
            funk( i, *items[ i ].data );
        }
    }

    void_t for_each( std::function< void_t( motor::string_in_t, T & ) > funk ) noexcept
    {
#if USE_DATAS_LOCKS
        motor::concurrent::mrsw_t::reader_lock_t lk( mtx );
#endif
        for( auto & i : items )
        {
            if( !i.valid ) continue;

#if USE_DATAS_LOCKS
            this_t::busy_guard __( items[ i ].gd );
#endif
            funk( i.name, *i.data );
        }
    }

    // if the user funk return false, the loop breaks,
    // otherwise if true, the loop continues
    // this function return true if the loop broke
    bool_t for_each_with_break( std::function< bool_t( T & ) > funk ) noexcept
    {
#if USE_DATAS_LOCKS
        motor::concurrent::mrsw_t::reader_lock_t lk( mtx );
#endif
        for( auto & i : items )
        {
            if( !i.valid ) continue;
#if USE_DATAS_LOCKS
            this_t::busy_guard __( items[ i ].gd );
#endif
            if( !funk( *i.data ) ) return true;
        }
        return false;
    }

    bool_t for_each_with_break( std::function< bool_t( motor::string_in_t, T & ) > funk ) noexcept
    {
#if USE_DATAS_LOCKS
        motor::concurrent::mrsw_t::reader_lock_t lk( mtx );
#endif
        for( auto & i : items )
        {
            if( !i.valid ) continue;
#if USE_DATAS_LOCKS
            this_t::busy_guard __( items[ i ].gd );
#endif
            if( !funk( i.name, *i.data ) ) return true;
        }
        return false;
    }

    bool_t for_each_with_break( std::function< bool_t( size_t const id, T & ) > funk ) noexcept
    {
#if USE_DATAS_LOCKS
        motor::concurrent::mrsw_t::reader_lock_t lk( mtx );
#endif
        for( size_t i = 0; i < items.size(); ++i )
        {
            if( !items[ i ].valid ) continue;
#if USE_DATAS_LOCKS
            this_t::busy_guard __( items[ i ].gd );
#endif
            if( !funk( i, *items[ i ].data ) ) return true;
        }
        return false;
    }

    bool_t for_each_with_break(
        std::function< bool_t( size_t const id, motor::string_in_t, T & ) > funk ) noexcept
    {
#if USE_DATAS_LOCKS
        motor::concurrent::mrsw_t::reader_lock_t lk( mtx );
#endif
        for( size_t i = 0; i < items.size(); ++i )
        {
            if( !items[ i ].valid ) continue;
#if USE_DATAS_LOCKS
            this_t::busy_guard __( items[ i ].gd );
#endif
            if( !funk( i, items[ i ].name, *items[ i ].data ) ) return true;
        }
        return false;
    }

  public: // unsave

    size_t size( void_t ) const noexcept
    {
        return items.size();
    }

  public:

    // invalidate everything and clear
    void invalidate_and_clear( void_t ) noexcept
    {
        for( auto & i : items )
        {
            if( i.valid )
            {
                i.data->invalidate( i.name );
            }
            motor::memory::global::dealloc< T >( motor::move( i.data ) );
        }
        items.clear();
    }

    // invalidate a particular item
    bool_t invalidate( size_t const oid ) noexcept
    {
#if USE_DATAS_LOCKS
        motor::concurrent::mrsw_t::reader_lock_t lk( mtx );
#endif
        if( oid >= items.size() ) return false;
        items[ oid ].data->invalidate( items[ oid ].name );
        items[ oid ].valid = false;
        items[ oid ].name.clear();
        return true;
    }
};
} // namespace platform
} // namespace motor