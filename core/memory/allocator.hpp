#pragma once

#include "global.h"

#include <iostream>

namespace motor
{
    namespace memory
    {
        template< typename T, typename mem_t = motor::memory::global >
        class allocator
        {
            typedef allocator< T, mem_t > __this_t ;
            motor_this_typedefs( __this_t ) ;

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

            allocator( void_t ) throw( ) { std::cout << "alloc" << std::endl ;}
            allocator( this_cref_t rhv ) throw( ) : _purpose( rhv._purpose ) {}
            allocator( this_rref_t rhv ) throw( ) : _purpose( ::std::move( rhv._purpose ) ) {}
            allocator( motor::memory::purpose_cref_t purpose ) throw( ) : _purpose( purpose ) {}

            template< typename U >
            allocator( allocator<U> const& rhv ) throw( ) : _purpose( rhv._purpose ) {}

            ~allocator( void_t ) {}

        public:

            bool_t operator == ( allocator const& rhv ) const
            {
                return _purpose == rhv._purpose ;
            }

            bool_t operator != ( allocator const& rhv ) const
            {
                return _purpose != rhv._purpose ;
            }

            template <class U>     
            this_ref_t operator=( allocator<U> const &) throw()
            {
                _purpose = rhv._purpose ;
                return *this ;
            }


            this_ref_t operator = ( this_cref_t rhv ) throw()
            {
                _purpose = rhv._purpose ;
                return *this ;
            }

            this_ref_t operator = ( this_rref_t rhv ) throw()
            {
                _purpose = std::move( rhv._purpose ) ;
                return *this ;
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

