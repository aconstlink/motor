#pragma once

#include "typedefs.h"
#include "global.h"
#include "../std/vector"

#include <motor/concurrent/mrsw.hpp>

#include <mutex>

namespace motor
{
    namespace memory
    {
        class arena_allocator
        {
        public:

            // @note will not compile if used
            template< typename T >
            using vector = motor::vector<T> ;

            template< typename T >
            static T * alloc_raw( char const * purpose ) noexcept
            {
                return motor::memory::global::alloc_raw<T>( purpose ) ;
            }

            template< typename T >
            static T * alloc_raw( size_t const n, char const * purpose ) noexcept 
            {
                return motor::memory::global::alloc_raw<T>( n, purpose ) ;
            }

            template< typename T >
            static void_t dealloc_raw( T * ptr ) noexcept 
            {
                motor::memory::global::dealloc_raw<T>( ptr ) ;
            }
        };

        namespace detail
        {
            // for internal usage. If the arena is used
            // within the memory manager itself so the 
            // memory manager can not be used.
            class arena_allocator
            {
            public:

                // @note will not compile if used
                template< typename T >
                using vector = std::vector<T> ;

                template< typename T >
                static T * alloc_raw( char const * ) noexcept
                {
                    return new( malloc( sizeof( T ) ) ) T() ;
                }

                template< typename T >
                static T * alloc_raw( size_t const n, char const * ) noexcept
                {
                    auto ret = reinterpret_cast<T*>( malloc( n * sizeof( T ) ) ) ;
                    for( size_t i=0; i<n; ++i )
                    {
                        new( ret+i ) T ;
                    }
                    return ret ;
                }

                template< typename T >
                static void_t dealloc_raw( T * ptr ) noexcept
                {
                    delete( ptr ) ;
                }
            };
        }
        

        template< typename T, typename allocator_t = motor::memory::arena_allocator >
        class arena
        {
            motor_this_typedefs( arena<T motor_comma allocator_t > ) ;
            motor_typedefs( T, type ) ;

            struct page
            {
                motor_this_typedefs( page ) ;

            public:

                std::mutex mtx ;
                size_t free_elems = 0 ;
                byte_ptr_t mem_ptr = nullptr ;
                
                size_t free_pos = 0 ;
                size_t * free_ptr = nullptr ;

            };
            motor_typedef( page ) ;

            class pages
            {
                motor_this_typedefs( pages ) ;

                size_t _pre_alloc = 10 ;

                size_t _size = 0 ;
                size_t _cap = 0 ;
                void_ptr_t * _pages = nullptr ;

            public:

                pages( void_t ) noexcept
                {
                    _pages = reinterpret_cast< void_ptr_t*>(malloc( sizeof( page_t*) * _pre_alloc )) ;
                    _size = _pre_alloc ;
                }

                pages( this_rref_t rhv ) noexcept
                {
                    this_t::clear() ;
                    _size = rhv._size ;
                    _cap = rhv._cap ;
                    _pre_alloc = rhv._pre_alloc ;
                    std::memcpy( _pages, rhv._pages, sizeof(page_ptr_t) * rhv._size ) ;
                }

                pages & operator = ( this_rref_t rhv ) noexcept
                {
                    this_t::clear() ;
                    _size = rhv._size ;
                    _cap = rhv._cap ;
                    _pre_alloc = rhv._pre_alloc ;
                    std::memcpy( _pages, rhv._pages, sizeof( page_ptr_t ) * rhv._size ) ;
                    return *this ;
                }

                void_t push_back( page_ptr_t ptr ) noexcept
                {
                    pages::resize( _pre_alloc ) ;

                    _pages[_cap++] = reinterpret_cast<void_ptr_t>( ptr ) ;
                }

                void_t clear( void_t ) noexcept
                {
                    free( _pages ) ;
                    _size = 0 ;
                    _cap = 0 ;
                    _pages = nullptr ;
                }

                using for_each_funk_t = std::function< bool_t ( size_t const, page_ptr_t ) > ;
                void_t for_each( for_each_funk_t f ) noexcept
                {
                    for( size_t i=0; i<_cap; ++i )
                    {
                        if( !f( i, reinterpret_cast<page_ptr_t>( _pages[i] ) ) ) break ;
                    }
                }

            private:

                void_t resize( size_t const pre_alloc ) noexcept
                {
                    if ( _cap < (_size-1) ) return ;
                    
                    void_ptr_t tmp = malloc( (_size + pre_alloc) * sizeof( page *) ) ;
                    std::memcpy( tmp, _pages, sizeof( page* )*_size ) ;
                    free( _pages ) ;
                    _pages = reinterpret_cast< void_ptr_t*>(tmp) ;

                }
            };
            motor_typedef( pages ) ;

            // currently, it is std and needs to stay like that
            // if this arena is used internally in the memory
            // manager itself.
            //motor_typedefs( std::vector< page_ptr_t >, pages ) ;

        private:

            size_t _prealloc = 100 ;

            motor::concurrent::mrsw_t _mtx ;
            pages_t _pages ;

        public:

            arena( void_t ) noexcept
            {
                this_t::alloc_page( _prealloc ) ;
            }

            arena( size_t const prealloc ) noexcept
            {
                _prealloc = prealloc ;
                this_t::alloc_page( _prealloc ) ;
            }

            arena( this_cref_t ) = delete ;
            arena( this_rref_t rhv ) noexcept : _prealloc( rhv._prealloc ),
                _pages( std::move( rhv._pages ) )
            {}

            ~arena( void_t ) noexcept
            {
                deinit() ;
            }

            this_ref_t operator = ( this_rref_t rhv ) noexcept
            {
                deinit() ;
                _prealloc = rhv._prealloc ;
                _pages = std::move( rhv._pages ) ;
                return *this ;
            }

            void_t deinit( void_t ) noexcept
            {
                _pages.for_each( [&]( size_t const, this_t::page_ptr_t p )
                {
                    allocator_t::dealloc_raw( p->mem_ptr ) ;
                    allocator_t::dealloc_raw( p->free_ptr ) ;
                    allocator_t::dealloc_raw( p ) ;
                    return true ;
                } ) ;
                _pages.clear() ;
            }

        public:

            //**********************************************************************************
            type_ptr_t alloc( type_rref_t rhv ) noexcept
            {
                page_ptr_t use_ptr = nullptr ;
                size_t index = size_t(-1) ;

                while( index == size_t(-1) )
                {
                    // 1. find a page with free memory
                    {
                        motor::concurrent::mrsw_t::reader_lock_t lk( _mtx ) ;
                        _pages.for_each( [&] ( size_t const, this_t::page_ptr_t p )
                        {
                            if ( p->free_elems > 0 )
                            {
                                use_ptr = p ;
                                return false ;
                            }
                            return true ;
                        } ) ;
                    }

                    // 2. if no page is found, allocate a new page
                    if ( use_ptr == nullptr )
                    {
                        use_ptr = this_t::alloc_page( _prealloc ) ;
                    }

                    // 3. get a free location
                    {
                        std::lock_guard< std::mutex > lk( use_ptr->mtx ) ;
                        
                        if( use_ptr->free_elems == 0 ) continue ;

                        size_t const free_pos = use_ptr->free_pos++ ;
                        use_ptr->free_elems-- ;

                        index = use_ptr->free_ptr[ free_pos ] ;
                    }
                }
                

                size_t const access_loc = index *
                    ( sizeof( T ) + sizeof( size_t ) + sizeof( page_ptr_t ) ) ;
    
                // 4. shift pointer for info storage
                byte_ptr_t ptr_start = use_ptr->mem_ptr + access_loc ;

                byte_ptr_t ptr_return = ptr_start + 
                    sizeof( size_t ) + sizeof( page_ptr_t ) ;

                // 5. store page pointer
                {
                    page_ptr_t * tmp_ptr = ( page_ptr_t * ) ptr_start ;
                    *tmp_ptr = use_ptr ;
                }

                // 6. store free pos
                {
                    size_t * tmp_ptr = ( size_t * ) (ptr_start+sizeof(page_ptr_t)) ;
                    *tmp_ptr = index ;
                }


                return new( ptr_return ) T( std::move( rhv ) ) ;
            }

            //**********************************************************************************
            type_ptr_t alloc( void_t ) noexcept
            {
                return this_t::alloc( type_t() ) ;
            }

            //**********************************************************************************
            void_t dealloc( type_ptr_t ptr ) noexcept
            {
                byte_ptr_t ptr_start = byte_ptr_t(ptr) - sizeof( size_t ) - sizeof(page_ptr_t) ;

                page_ptr_t page_ptr = *(( page_ptr_t * ) ptr_start) ;
                size_t const index = *( ( size_t* ) ( ptr_start + sizeof( page_ptr_t ) ) ) ;

                ptr->~type_t() ;

                // return memory
                {
                    std::lock_guard< std::mutex > lk( page_ptr->mtx ) ;
                    page_ptr->free_elems++ ;
                    page_ptr->free_pos = page_ptr->free_pos - 1 ;
                    page_ptr->free_ptr[ page_ptr->free_pos ] = index ;
                }
            }

        private:

            //**********************************************************************************
            page_ptr_t alloc_page( size_t const prealloc ) noexcept
            {
                auto * p = allocator_t::alloc_raw<page_t>( "[arena::alloc_page] : page" ) ;

                // 1. the memory block itself
                {
                    size_t const sib = prealloc * sizeof( T ) ;
                    size_t const sibx = prealloc * ( sizeof( size_t ) << 1 ) ;

                    byte_ptr_t raw_mem = allocator_t::alloc_raw<byte_t>( sib + sibx,
                        "[motor::memory::arena] : raw memory data" ) ;

                    p->mem_ptr = raw_mem ;
                }

                // 2. the freelist
                {
                    size_t const sib = prealloc * sizeof( size_t ) ;
                    byte_ptr_t raw_mem = allocator_t::alloc_raw<byte_t>( sib,
                        "[motor::memory::arena] : raw memory freelist" ) ;

                    p->free_ptr = (size_t*)raw_mem ;

                    {
                        size_t * ptr = ( size_t* ) raw_mem ;
                        for( size_t i = 0; i < prealloc; ++i )
                        {
                            ptr[ i ] = i ;
                        }
                    }
                }

                // 3. properties
                {
                    p->free_elems = prealloc ;
                    p->free_pos = 0 ;
                }

                {
                    //std::lock_guard< std::mutex > lk( _mtx ) ;
                    motor::concurrent::mrsw_t::writer_lock_t lk( _mtx ) ;
                    _pages.push_back( p ) ;
                }

                return p ;
            }
        };
    }
}
