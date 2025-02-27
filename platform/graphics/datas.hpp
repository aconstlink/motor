
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

        class try_guard
        {
        public:

            using atomic_t = std::atomic< size_t > ;

        private:

            atomic_t & _what ;
            bool_t _accessed = false ;

        public:
            
            try_guard( atomic_t & w ) noexcept : _what(w) {}
            ~try_guard( void_t ) noexcept 
            {
                if( _accessed ) _what = 0 ;
            }

            bool_t now( void_t ) noexcept
            {
                size_t expected = 0 ;
                _accessed = _what.compare_exchange_weak( expected, 1 ) ;
                return _accessed ;
            }
        };

        class busy_guard
        {
        private:

            std::atomic< size_t > & _what ;

        public:

            busy_guard( std::atomic< size_t > & w ) noexcept : _what( w )
            {
                size_t expected = 0 ;
                while( !_what.compare_exchange_weak( expected, size_t(1) ) ) ;
            }

            ~busy_guard( void_t ) noexcept
            {
                _what = 0 ;
            }
        };
        
    private:

        struct wrapper
        {
            //motor::concurrent::mrsw_t busy_mtx ;
            //bool_t busy = false ;
            //std::atomic< bool_t > busy = false ;
            std::atomic< size_t > busy = 0 ;


            bool_t valid = false ;
            motor::string_t name ;

            T * data = nullptr ;

            wrapper( void_t ) noexcept{}

            wrapper( wrapper && rhv ) noexcept : busy( false ),
                valid( rhv.valid), name( std::move( rhv.name) ), data( motor::move( rhv.data ) )
            {}

            wrapper & operator = ( wrapper && rhv ) noexcept 
            {
                busy = false ;
                valid = rhv.valid ;
                rhv.valid = false ;
                name = std::move( rhv.name ) ;
                data = motor::move( rhv.data ) ;
                return *this ;
            }
        };

        using items_t = motor::vector< wrapper > ;

        mutable motor::concurrent::mrsw_t mtx ;
        items_t items ;

    public:

        datas( void_t ) noexcept {}
        datas( this_cref_t ) = delete ;
        datas( this_rref_t rhv ) noexcept : items( std::move( rhv.items ) ) {}
        this_ref_t operator = ( this_cref_t ) = delete ;
        this_ref_t operator = ( this_rref_t rhv ) noexcept
        {
            items = std::move( rhv.items ) ;
            return *this ;
        }
        ~datas( void_t ) noexcept
        {
            this_t::invalidate_and_clear() ;
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

                this_t::busy_guard busy_wait( items[oid].busy ) ;
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

    private: // internal access

        std::pair< bool_t, bool_t > try_access( size_t oid, std::function< void_t ( this_t::wrapper & ) > funk ) noexcept
        {
            if( oid == size_t(-1) ) return std::make_pair( false, false ) ;

            {
                motor::concurrent::mrsw_t::reader_lock_t lk( mtx ) ;
                
                if( this_t::check_oid( oid, items ) == size_t( -1 ) ) 
                    return std::make_pair( false, false ) ;
                
                this_t::try_guard try_lock( items[oid].busy ) ;
                if( !try_lock.now() )
                    return std::make_pair( true, false ) ;

                funk( items[ oid ] ) ;
            }
            return std::make_pair( false, true ) ;
        }

    public: // try access

        std::pair< bool_t, bool_t > try_access( size_t oid, std::function< void_t ( T & ) > funk ) noexcept
        {
            return this_t::try_access( oid, [&]( this_t::wrapper & wrp )
            {
                funk( *wrp.data ) ;
            } ) ;
        }

        std::pair< bool_t, bool_t > try_access( size_t oid, std::function< void_t ( motor::string_in_t, T & ) > funk ) noexcept
        {
            return this_t::try_access( oid, [&]( this_t::wrapper & wrp )
            {
                funk( wrp.name, *wrp.data ) ;
            } ) ;
        }

        // access by id only
        // if you have an id and you know the object exists, use this function
        template< typename R >
        std::pair< bool_t, R > try_access( size_t const oid, std::function< R ( T & ) > funk ) noexcept
        {
            if( oid == size_t(-1) ) return std::make_pair( false, R() ) ; 

            motor::concurrent::mrsw_t::reader_lock_t lk( mtx ) ;
                
            if( this_t::check_oid( oid, items ) == size_t( -1 ) ) 
                return std::make_pair( false, R() ) ;
               
            this_t::try_guard try_lock( items[oid].busy ) ;
            if( !try_lock.now() )
                return std::make_pair( true, R() ) ;

            return std::make_pair( true, funk( *items[ oid ].data ) ) ;
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

                this_t::busy_guard busy_wait( items[oid].busy ) ;
                funk( items[ oid ].name, *items[ oid ].data ) ;
                
            }
            return true ;
        }
        
        bool_t access_by_name( motor::string_in_t name, std::function< void_t ( size_t const, T & ) > funk ) noexcept
        {
            motor::concurrent::mrsw_t::reader_lock_t lk( mtx ) ;
            
            auto i=size_t(-1) ;
            while( ++i < items.size() && items[i].name != name ) ;

            if( i == items.size() ) return false ;

            this_t::busy_guard busy_wait( items[i].busy ) ;
            funk( i, *items[ i ].data ) ;

            return true ;
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
               
            this_t::busy_guard busy_wait( items[oid].busy ) ;

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

            if ( oid >= v.size() && 
                v.size() == v.capacity() )
            {
                oid = v.size() ;
                v.reserve( oid + 20 ) ;
            }

            if( oid == v.size() )
            {
                this_t::wrapper w ;
                w.busy = false ;
                w.valid = true ;
                w.name = name ;
                w.data = motor::memory::global::alloc<T>( typeid(T).name() ) ;

                v.emplace_back( std::move( w ) ) ;
            }
            else
            {
                v[ oid ].valid = true ;
                v[ oid ].name = name ;
            }
            

            return oid ;
        }

    public:

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

                this_t::busy_guard busy_wait( i.busy ) ;
                funk( *i.data ) ;
            }
        }

        void_t for_each( std::function < void_t ( size_t const, T & ) > funk ) noexcept
        {
            motor::concurrent::mrsw_t::reader_lock_t lk( mtx ) ;
            for( size_t i=0; i<items.size(); ++i ) 
            {
                if( !items[i].valid ) continue ;

                this_t::busy_guard busy_wait( items[i].busy ) ;
                funk( i, *items[i].data ) ;
            }
        }

        void_t for_each( std::function < void_t ( motor::string_in_t, T & ) > funk ) noexcept
        {
            motor::concurrent::mrsw_t::reader_lock_t lk( mtx ) ;
            for( auto & i : items ) 
            {
                if( !i.valid ) continue ;

                this_t::busy_guard __( items[i].busy ) ;
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
                this_t::busy_guard __( items[i].busy ) ;
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
                this_t::busy_guard __( items[i].busy ) ;
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
                this_t::busy_guard __( items[i].busy ) ;
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
                this_t::busy_guard __( items[i].busy ) ;
                if( !funk( i, items[i].name, *items[i].data ) ) return true ;
            }
            return false ;
        }

    public: // unsave
       

        size_t size( void_t ) const noexcept
        {
            return items.size() ;
        }

    public:

        // invalidate everything and clear
        void invalidate_and_clear( void_t ) noexcept
        {
            for( auto & i : items ) 
            {
                if( i.valid ) 
                {
                    i.data->invalidate( i.name ) ;
                }
                motor::memory::global::dealloc<T>( 
                    motor::move( i.data ) ) ;
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