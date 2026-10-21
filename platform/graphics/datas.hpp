// SPDX-FileCopyrightText: 2023-2026 Alex Link
// SPDX-License-Identifier: MIT

#pragma once

#include "../typedefs.h"
#include <motor/concurrent/mrsw.hpp>

namespace motor
{
namespace platform
{
//*******************************************************************************************
template < typename T >
struct datas
{
    motor_this_typedefs( datas< T > );

  public:

    class handle
    {
        motor_this_typedefs( handle );

      private:

        size_t _oid = size_t( -1 );
        datas< T > & _owner;

      public:

        handle( datas< T > & owner ) noexcept : _owner( owner ) {}
        handle( size_t const oid, datas< T > & owner ) noexcept : _oid( oid ), _owner( owner ) {}

        handle( this_cref_t rhv ) noexcept : _oid( rhv._oid ), _owner( rhv._owner ) {}
        handle( this_rref_t rhv ) noexcept : _oid( rhv._oid ), _owner( rhv._owner )
        {
            if( &rhv != this ) rhv._oid = size_t( -1 );
        }

        ~handle( void_t ) noexcept {}

      public:

        size_t oid( void_t ) const noexcept
        {
            return _oid;
        }

        motor::string_cref_t name( void_t ) const noexcept
        {
            return _owner.items[ _oid ].name;
        }

        T * operator->( void_t ) noexcept
        {
            return _owner.items[ _oid ].data;
        }

        T & operator*( void_t ) noexcept
        {
            return *_owner.items[ _oid ].data;
        }

        T const & operator*( void_t ) const noexcept
        {
            return *_owner.items[ _oid ].data;
        }

        operator bool( void_t ) const noexcept
        {
            return check_valid( _oid );
        }

        bool_t check_valid( size_t const oid ) const noexcept
        {
            return oid < _owner.items.size() && _owner.items[ oid ].valid;
        }
    };
    motor_typedef( handle );

    class checked_handle
    {
        motor_this_typedefs( checked_handle );

      private:

        handle _handle;

      public:

        explicit checked_handle( handle const & hnd ) noexcept : _handle( hnd )
        {
            assert( hnd && "handle must be valid" );
        }

        checked_handle( checked_handle const & ) noexcept = default;

        motor::string_cref_t name( void_t ) const noexcept
        {
            return _handle.name();
        }

        T * operator->( void_t ) noexcept
        {
            return _handle.operator->();
        }

        T & operator*( void_t ) noexcept
        {
            return *_handle;
        }

        size_t oid( void_t ) const noexcept
        {
            return _handle.oid();
        }
    };
    motor_typedef( checked_handle );

    friend class handle;

  private:

    struct wrapper
    {
        bool_t valid = false;
        motor::string_t name;

        T * data = nullptr;

        wrapper( void_t ) noexcept {}
        wrapper( wrapper const & ) = delete;

        wrapper( wrapper && rhv ) noexcept
            : valid( rhv.valid ), name( std::move( rhv.name ) ), data( motor::move( rhv.data ) )
        {
        }

        wrapper & operator=( wrapper && rhv ) noexcept
        {
            valid = rhv.valid;
            rhv.valid = false;
            name = std::move( rhv.name );
            data = motor::move( rhv.data );
            return *this;
        }
    };

    using items_t = motor::vector< wrapper >;

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

  public: 

    ////////////////////////////////////////////////////////////////////////////
    // NEW API with handles
    ////////////////////////////////////////////////////////////////////////////

    // will create an entry with the name, if non exist. if the oid is wrong
    // the oid will be set to the oid holding the name
    this_t::handle_t access( size_t oid, motor::string_in_t name ) noexcept
    {
        // #1 quick check if id is still valid
        {
            oid = this_t::check_oid( oid, items );
        }

        // #2 if new or invalid, create new oid
        if( oid == size_t( -1 ) )
        {
            oid = this_t::determine_oid( oid, name, items );
        }

        // #3 oid is safe, can access now.
        if( oid != size_t( -1 ) )
        {
            return this_t::handle_t( oid, *this );
        }

        return this_t::handle_t( *this );
    }

    // will try accessing the resource with oid. if it does not
    // exist, the function returns an invalid handle.
    this_t::handle_t access( size_t const oid ) noexcept
    {
        auto const cond = oid == size_t( -1 ) || this_t::check_oid( oid, items ) == size_t( -1 );
        return cond ? this_t::handle_t( *this ) : this_t::handle_t( oid, *this );
    }

    void_t for_each( std::function< void_t( checked_handle_t ) > funk ) noexcept
    {
        size_t i = size_t(-1) ;
        for( auto & item : items )
        {
            auto hnd = this_t::handle_t( ++i, *this ) ;
            if( !hnd ) continue ;

            funk( this_t::checked_handle_t( hnd ) ) ;
        }
    } 

    ////////////////////////////////////////////////////////////////////////////
    // OLD API
    ////////////////////////////////////////////////////////////////////////////

  public: // read and write access

    // @obsolete
    // access if you are not sure if the object exists.
    // this function will also create that object
    bool_t access( size_t & oid, motor::string_in_t name,
        std::function< bool_t( size_t const, motor::string_in_t, T & ) > funk ) noexcept
    {
        // #1 quick check if id is still valid
        {
            oid = this_t::check_oid( oid, items );
        }

        // #2 if new or invalid, create new oid
        if( oid == size_t( -1 ) )
        {
            oid = this_t::determine_oid( oid, name, items );
        }

        // #3 oid is safe, can access now.
        if( oid != size_t( -1 ) )
        {
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
            if( this_t::check_oid( oid, items ) == size_t( -1 ) )
                return std::make_pair( false, false );

            funk( items[ oid ] );
        }
        return std::make_pair( false, true );
    }

  public: // try access

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

        if( this_t::check_oid( oid, items ) == size_t( -1 ) ) return std::make_pair( false, R() );

#if USE_DATAS_LOCKS
        this_t::try_guard try_lock( items[ oid ].gd );
        if( !try_lock.has_accessed() ) return std::make_pair( true, R() );
#endif
        return std::make_pair( true, funk( *items[ oid ].data ) );
    }

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
            if( this_t::check_oid( oid, items ) == size_t( -1 ) ) return false;
            funk( items[ oid ].name, *items[ oid ].data );
        }
        return true;
    }

    bool_t access_by_name(
        motor::string_in_t name, std::function< void_t( size_t const, T & ) > funk ) noexcept
    {
        auto i = size_t( -1 );
        while( ++i < items.size() && items[ i ].name != name );

        if( i == items.size() ) return false;
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
        if( this_t::check_oid( oid, items ) == size_t( -1 ) ) return std::make_pair( false, R() );
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

        size_t i = size_t( -1 );
        while( ++i < items.size() && items[ i ].name != name );
        return i == items.size() ? size_t( -1 ) : i;
    }

    void_t for_each( std::function< void_t( T & ) > funk ) noexcept
    {
        for( auto & i : items )
        {
            if( !i.valid ) continue;
            funk( *i.data );
        }
    }

    void_t for_each( std::function< void_t( size_t const, T & ) > funk ) noexcept
    {
        for( size_t i = 0; i < items.size(); ++i )
        {
            if( !items[ i ].valid ) continue;
            funk( i, *items[ i ].data );
        }
    }

    void_t for_each( std::function< void_t( motor::string_in_t, T & ) > funk ) noexcept
    {
        for( auto & i : items )
        {
            if( !i.valid ) continue;
            funk( i.name, *i.data );
        }
    }

    // if the user funk return false, the loop breaks,
    // otherwise if true, the loop continues
    // this function return true if the loop broke
    bool_t for_each_with_break( std::function< bool_t( T & ) > funk ) noexcept
    {
        for( auto & i : items )
        {
            if( !i.valid ) continue;
            if( !funk( *i.data ) ) return true;
        }
        return false;
    }

    bool_t for_each_with_break( std::function< bool_t( motor::string_in_t, T & ) > funk ) noexcept
    {
        for( auto & i : items )
        {
            if( !i.valid ) continue;
            if( !funk( i.name, *i.data ) ) return true;
        }
        return false;
    }

    bool_t for_each_with_break( std::function< bool_t( size_t const id, T & ) > funk ) noexcept
    {
        for( size_t i = 0; i < items.size(); ++i )
        {
            if( !items[ i ].valid ) continue;
            if( !funk( i, *items[ i ].data ) ) return true;
        }
        return false;
    }

    bool_t for_each_with_break(
        std::function< bool_t( size_t const id, motor::string_in_t, T & ) > funk ) noexcept
    {
        for( size_t i = 0; i < items.size(); ++i )
        {
            if( !items[ i ].valid ) continue;
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
        if( oid >= items.size() ) return false;
        items[ oid ].data->invalidate( items[ oid ].name );
        items[ oid ].valid = false;
        items[ oid ].name.clear();
        return true;
    }
};
} // namespace platform
} // namespace motor