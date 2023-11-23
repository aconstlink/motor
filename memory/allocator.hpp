#pragma once

#include "global.h"

#include <iostream>

namespace motor
{
    namespace memory
    {
        // C++ 11 allocator. Need new one for C++ 17 and C++ 20 due to 
        // deprecated functionality
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

            char_cptr_t _purpose = nullptr ;

        public:

            allocator( void_t ) throw( ) {}
            allocator( this_cref_t rhv ) throw( ) : _purpose( rhv._purpose ) {}
            allocator( this_rref_t rhv ) throw( ) : _purpose( std::move( rhv._purpose ) ) {}
            allocator( char_cptr_t purpose ) throw( ) : _purpose( purpose ) {}

            template< typename U >
            allocator( allocator<U> const& rhv ) throw( ) : _purpose( rhv._purpose ) {}
            
            ~allocator( void_t ) {}

            bool_t operator == ( allocator const& rhv ) const
            {
                return _purpose == rhv._purpose ;
            }

            bool_t operator != ( allocator const& rhv ) const
            {
                return _purpose != rhv._purpose ;
            }

        public:

            pointer allocate( size_type n )
            {
                //std::cout << "alloc of n = " << std::to_string(n) << std::endl ;
                return mem_t:: template alloc_raw<value_type>( n, _purpose ) ;
            }

            void_t deallocate( pointer ptr, size_type /*n*/ )
            {
                //std::cout << "dealloc of n = " << std::to_string(n) << std::endl ;
                mem_t::dealloc_raw(ptr) ;
            }

            size_type max_size( void_t ) const throw( )
            {
                return size_type( -1 ) ;
            }
        };
    }
}

