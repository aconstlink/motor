
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

        struct wrapper
        {
            //motor::concurrent::mrsw_t busy_mtx ;
            bool_t busy = false ;

            bool_t valid = false ;
            motor::string_t name ;

            T * data ;
        };

        using items_t = motor::vector< wrapper > ;

        mutable motor::concurrent::mrsw_t mtx ;
        items_t items ;

    public:

        datas( void_t ) noexcept {}
        datas( this_rref_t rhv ) noexcept : items( std::move( rhv.items ) ) {}
        this_ref_t operator = ( this_rref_t rhv ) noexcept
        {
            items = std::move( rhv.items ) ;
            return *this ;
        }
        ~datas( void_t ) noexcept
        {
            for( auto & i : items )
            {
                motor::memory::global::dealloc<T>( motor::move( i.data ) ) ;
            }
        }

    public: // read and write access

        // access if you are not sure if the object exists.
        // this function will also create that object
        bool_t access( size_t & oid, motor::string_in_t name, 
            std::function< bool_t ( size_t const, motor::string_in_t, T & ) > funk ) noexcept
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
                return funk( oid, items[ oid ].name, *items[ oid ].data ) ;
            }

            return false ;
        }

        // access if you are not sure if the object exists.
        // this function will also create that object
        bool_t access( size_t & oid, motor::string_in_t name, 
            std::function< bool_t ( size_t const, T & ) > funk ) noexcept
        {
            return this_t::access( oid, name, [&]( size_t const, motor::string_in_t name_, T & d )
            {
                return funk( oid, d ) ;
            } ) ;
        }

        // access if you are not sure if the object exists.
        // this function will also create that object
        bool_t access( size_t & oid, motor::string_in_t name, 
            std::function< bool_t ( motor::string_in_t, T & ) > funk ) noexcept
        {
            return this_t::access( oid, name, [&]( size_t const, motor::string_in_t name_, T & d )
            {
                return funk( name_, d ) ;
            } ) ;
        }

        bool_t access( size_t & oid, motor::string_in_t name, 
            std::function< bool_t ( T & ) > funk ) noexcept
        {
            return this_t::access( oid, name, [&]( size_t const, motor::string_in_t, T & d )
            {
                return funk( d ) ;
            } ) ;
        }

    public: // read access

        bool_t access( size_t oid, std::function< void_t ( T & ) > funk ) noexcept
        {
            return this_t::access( oid, [=]( motor::string_in_t, T & d )
            {
                return funk( d ) ;
            } ) ;
        }

        bool_t access( size_t oid, std::function< void_t ( motor::string_in_t name, T & ) > funk ) noexcept
        {
            if( oid == size_t(-1) ) return false ;

            {
                motor::concurrent::mrsw_t::reader_lock_t lk( mtx ) ;
                
                if( this_t::check_oid( oid, items ) == size_t( -1 ) ) 
                    return false ;

                funk( items[ oid ].name, *items[ oid ].data ) ;
            }
            return true ;
        }

        bool_t access_by_name( motor::string_cref_t name, std::function< void_t ( size_t const id, T & ) > funk ) noexcept
        {
            bool_t res = false ;
            this_t::for_each_with_break( [&]( size_t const id, motor::string_in_t name_, T & d )
            {
                if( name_ == name ) 
                {
                    funk( id, d ) ;
                    res = true ;
                    return false ;
                }
                return true ;
            } ) ;

            return res ;
        }

        // access by id only
        // if you have an id and you know the object exists, use this function
        template< typename R >
        std::pair< bool_t, R > access( size_t const oid, std::function< R ( motor::string_in_t, T & ) > funk ) noexcept
        {
            if( oid == size_t(-1) ) return std::make_pair( false, R() ) ; 

            motor::concurrent::mrsw_t::reader_lock_t lk( mtx ) ;
                
            if( this_t::check_oid( oid, items ) == size_t( -1 ) ) 
                return std::make_pair( false, R() ) ;
               
            return std::make_pair( true, funk( items[oid].name, *items[ oid ].data ) ) ;
        }

        // access by id only
        // if you have an id and you know the object exists, use this function
        template< typename R >
        std::pair< bool_t, R > access( size_t const oid, std::function< R ( T & ) > funk ) noexcept
        {
            return this_t::access<R>( oid, [=]( motor::string_in_t, T & d )
            {
                return funk( d ) ;
            } ) ;
        }
        
        

    private:

        static size_t check_oid( size_t const oid, this_t::items_t & v ) noexcept
        {
            return oid < v.size() ? oid : size_t(-1) ;
        }

        static size_t determine_oid( size_t oid, motor::string_cref_t name, this_t::items_t & v ) noexcept
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
            v[ oid ].data = motor::memory::global::alloc<T>() ;

            return oid ;
        }

    public:

        // find by name
        bool_t find( motor::string_in_t name, std::function< void_t ( size_t const, T & ) > funk ) noexcept
        {
            motor::concurrent::mrsw_t::reader_lock_t lk( mtx ) ;
            
            auto i=size_t(-1) ;
            while( ++i < items.size() && items[i].name != name ) ;

            if( i == items.size() ) return false ;

            funk( i, *items[ i ].data ) ;

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
            for( auto & i : items ) 
            {
                if( !i.valid ) continue ;
                funk( *i.data ) ;
            }
        }

        void_t for_each( std::function < void_t ( size_t const, T & ) > funk ) noexcept
        {
            motor::concurrent::mrsw_t::reader_lock_t lk( mtx ) ;
            for( size_t i=0; i<items.size(); ++i ) 
            {
                if( !items[i].valid ) continue ;
                funk( i, *items[i].data ) ;
            }
        }

        void_t for_each( std::function < void_t ( motor::string_in_t, T & ) > funk ) noexcept
        {
            motor::concurrent::mrsw_t::reader_lock_t lk( mtx ) ;
            for( auto & i : items ) 
            {
                if( !i.valid ) continue ;
                funk( i.name, *i.data ) ;
            }
        }

        // if the user funk return false, the loop breaks, 
        // otherwise if true, the loop continues
        // this function return true if the loop broke
        bool_t for_each_with_break( std::function < bool_t ( T & ) > funk ) noexcept
        {
            motor::concurrent::mrsw_t::reader_lock_t lk( mtx ) ;
            for( auto & i : items ) 
            {
                if( !i.valid ) continue ;
                if( !funk( *i.data ) ) return true ;
            }
            return false ;
        }

        bool_t for_each_with_break( std::function < bool_t ( motor::string_in_t, T & ) > funk ) noexcept
        {
            motor::concurrent::mrsw_t::reader_lock_t lk( mtx ) ;
            for( auto & i : items ) 
            {
                if( !i.valid ) continue ;
                if( !funk( i.name, *i.data ) ) return true ;
            }
            return false ;
        }

        bool_t for_each_with_break( std::function < bool_t ( size_t const id, T & ) > funk ) noexcept
        {
            motor::concurrent::mrsw_t::reader_lock_t lk( mtx ) ;
            for( size_t i=0; i<items.size(); ++i )
            {
                if( !items[i].valid ) continue ;
                if( !funk( i, *items[i].data ) ) return true ;
            }
            return false ;
        }

        bool_t for_each_with_break( std::function < bool_t ( size_t const id, motor::string_in_t, T & ) > funk ) noexcept
        {
            motor::concurrent::mrsw_t::reader_lock_t lk( mtx ) ;
            for( size_t i=0; i<items.size(); ++i )
            {
                if( !items[i].valid ) continue ;
                if( !funk( i, items[i].name, *items[i].data ) ) return true ;
            }
            return false ;
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
            for( auto & i : items ) 
            {
                if( !i.valid ) continue ;
                i.data->invalidate( i.name ) ;
            }
            items.clear() ;
        }

        // invalidate a particular item
        bool_t invalidate( size_t const oid ) noexcept
        {
            motor::concurrent::mrsw_t::reader_lock_t lk( mtx ) ;
            if( oid >= items.size() ) return false ;
            items[oid].data->invalidate( items[oid].name ) ;
            items[oid].valid = false ;
            items[oid].name.clear() ;
            return true ;
        }
    } ;
    }
}