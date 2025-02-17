
#pragma once

#include "../typedefs.h"
#include <motor/concurrent/mrsw.hpp>

namespace motor
{
    namespace platform
    {
        //*******************************************************************************************
    template< typename T >
    struct datas
    {
        motor_this_typedefs( datas< T > ) ;

    private:

        mutable motor::concurrent::mrsw_t mtx ;
        motor::vector< T > items ;

    public:

        datas( void_t ) noexcept {}
        datas( this_rref_t rhv ) noexcept : items( std::move( rhv.items ) ) {}
        this_ref_t operator = ( this_rref_t rhv ) noexcept
        {
            items = std::move( rhv.items ) ;
            return *this ;
        }

        // access if you are not sure if the object exists.
        // this function will also create that object
        bool_t access( size_t & oid, motor::string_in_t name, std::function< bool_t ( T & ) > funk ) noexcept
        {
            // #1 quick check if id is still valid
            {
                motor::concurrent::mrsw_t::reader_lock_t lk( mtx ) ;
                oid = this_t::check_oid( oid, items ) ;
            }

            // #2 if new or invalid, create new oid
            if ( oid == size_t( -1 ) )
            {
                motor::concurrent::mrsw_t::writer_lock_t lk( mtx ) ;
                oid = this_t::determine_oid( oid, name, items ) ;
            }

            // #3 oid is safe, can access now.
            if( oid != size_t( -1 ) )
            {
                motor::concurrent::mrsw_t::reader_lock_t lk( mtx ) ;
                return funk( items[ oid ] ) ;
            }

            return false ;
        }

        // access by id only
        // if you have an id and you know the object exists, use this function
        bool_t access( size_t oid, std::function< void_t ( T & ) > funk ) noexcept
        {
            {
                motor::concurrent::mrsw_t::reader_lock_t lk( mtx ) ;
                
                if( this_t::check_oid( oid, items ) == size_t( -1 ) ) 
                    return false ;
               
                funk( items[ oid ] ) ;
            }

            return true ;
        }
        
    private:

        static size_t check_oid( size_t const oid, motor::vector< T > & v ) noexcept
        {
            return oid < v.size() ? oid : size_t(-1) ;
        }

        static size_t determine_oid( size_t oid, motor::string_cref_t name, motor::vector< T > & v ) noexcept
    {
        if( this_t::check_oid( oid, v ) != size_t(-1) ) return oid ;
        if( name.empty() ) return oid ;

        assert( oid == size_t(-1) && "must be -1. if oid != -1 => object MUST be valid!" ) ;

        // we have to search for the name first. This can happen
        // if a shader/msl object is recompiled.
        {
            while ( ++oid < v.size() && v[ oid ].name != name ) ;
            if ( oid < v.size() ) return oid ;
        }

        // look for the next invalid item to be reused.
        {
            oid = size_t( -1 ) ;
            while ( ++oid < v.size() && v[ oid ].valid ) ;
        }

        if ( oid >= v.size() )
        {
            oid = v.size() ;
            v.resize( oid + 10 ) ;
        }

        v[ oid ].valid = true ;
        v[ oid ].name = name ;

        return oid ;
    }

    public:

        size_t validate( size_t const oid ) noexcept
        {
            motor::concurrent::mrsw_t::reader_lock_t lk( mtx ) ;
            return pimpl::check_oid( oid, items ) ;
        }

        // find by name
        bool_t find( motor::string_in_t name, std::function< void_t ( size_t const, T & ) > funk ) noexcept
        {
            motor::concurrent::mrsw_t::reader_lock_t lk( mtx ) ;
            
            auto i=size_t(-1) ;
            while( ++i < items.size() && items[i].name != name ) ;

            if( i == items.size() ) return false ;

            funk( i, items[ i ] ) ;

            return true ;
        }

        // find render object by name
        bool_t find_ro( motor::string_in_t name, std::function< void_t ( size_t const, T & ) > funk ) noexcept
        {
            motor::concurrent::mrsw_t::reader_lock_t lk( mtx ) ;

            size_t j = 0;
            for(j; j<items.size(); ++j ) 
            {
                auto i = size_t( -1 ) ;
                while ( ++i < items[j].ros.size() && items[ j ].ros[i].name() != name ) ;
                if( i != items[j].ros.size() ) break ;
            }
            
            if( j == items.size() ) return false ;

            funk( j, items[ j ] ) ;

            return true ;
        }

        size_t find_by_name( motor::string_in_t name ) const noexcept
        {
            if( name.empty() ) return size_t(-1) ;

            motor::concurrent::mrsw_t::reader_lock_t lk( mtx ) ;

            size_t i = size_t(-1) ;
            while( ++i < items.size() && items[i].name != name ) ;
            return i == items.size() ? size_t(-1) : i ;
        }

        void_t for_each( std::function < void_t ( T & ) > funk ) noexcept
        {
            motor::concurrent::mrsw_t::reader_lock_t lk( mtx ) ;
            for( auto & i : items ) funk( i ) ;
        }

    public: // unsave

        #if 0
        T & operator[] ( size_t const idx ) noexcept
        {
            return items[idx] ;
        }
        #endif

        size_t size( void_t ) const noexcept
        {
            return items.size() ;
        }

        void_t resize( size_t const ni ) noexcept
        {
            motor::concurrent::mrsw_t::writer_lock_t lk( mtx ) ;
            items.resize( ni ) ;
        }

    public:

        // invalidate everything and clear
        void invalidate_and_clear( void_t ) noexcept
        {
            for( auto & i : items ) i.invalidate() ;
            items.clear() ;
        }

        // invalidate a particular item
        bool_t invalidate( size_t const oid ) noexcept
        {
            motor::concurrent::mrsw_t::reader_lock_t lk( mtx ) ;
            if( oid >= items.size() ) return false ;
            items[oid].invalidate() ;
            return true ;
        }
    } ;
    }
}