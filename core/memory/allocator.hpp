#pragma once

#include "global.h"

namespace motor
{
    namespace memory
    {
        template< typename T, typename mem_t = motor::memory::global >
        class allocator
        {
        public:

            typedef T value_type ;
            typedef T* pointer ;
            typedef T& reference ;
            typedef const T* const_pointer ;
            typedef const T& const_reference ;
            typedef size_t size_type ;
            typedef ptrdiff_t difference_type ;

            template <class U> struct rebind { typedef allocator<U> other; };

            motor::memory::purpose_t _purpose = "[motor::memory] : allocator" ;

        public:

            allocator( void_t ) throw( ) {}
            allocator( allocator const& rhv ) throw( ) : _purpose( rhv._purpose ) {}
            allocator( allocator&& rhv ) throw( ) : _purpose( ::std::move( rhv._purpose ) ) {}
            allocator( motor::memory::purpose_cref_t purpose ) throw( ) : _purpose( purpose ) {}

            template< typename U >
            allocator( allocator<U> const& /*rhv*/ ) throw( ) {}

        public:

            bool_t operator == ( allocator const& rhv ) const
            {
                return _purpose == rhv._purpose ;
            }

            bool_t operator != ( allocator const& rhv ) const
            {
                return _purpose != rhv._purpose ;
            }

        public:

            pointer address( reference x ) const
            {
                mem_t::address( x ) ;
            }

            const_pointer address( const_reference x ) const
            {
                mem_t::address( x ) ;
            }

            pointer allocate( size_type n, void_ptr_t hint = nullptr )
            {
                if( hint != nullptr ) mem_t::dealloc( hint ) ;
                return mem_t:: template alloc<value_type>( n, _purpose ) ;
            }

            void_t deallocate( pointer ptr, size_type /*n*/ )
            {
                mem_t::dealloc( ptr ) ;
            }

            size_type max_size( void_t ) const throw( )
            {
                return size_type( -1 ) ;
            }

            void_t construct( pointer p, const_reference val )
            {
                new( p )T( val ) ;
            }

            void_t destroy( pointer p )
            {
                ( *p ).~T() ;
            }
        };
    }
}

