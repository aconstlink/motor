#pragma once

#include "../api.h"
#include "typedefs.h"
#include <mutex>

namespace motor
{
    namespace memory
    {
        class MOTOR_CORE_API global
        {
            motor_this_typedefs( global ) ;

        private:

            motor::memory::imanager_ptr_t _manager ;

        private: // singleton

            static std::mutex _mtx ;
            static this_ptr_t _ptr ;

        public:

            global( void_t ) ;
            global( this_cref_t ) = delete ;
            global( this_rref_t ) ;
            ~global( void_t ) ;

        public: // singleton functions

            static this_ptr_t init( void_t ) ;
            static void_t deinit( void_t ) ;

            static this_ptr_t get( void_t ) ;


            static void_ptr_t alloc( size_t sib, motor::memory::purpose_cref_t purpose ) ;
            static void_ptr_t alloc( size_t sib ) ;
            static void_ptr_t dealloc( void_ptr_t ) ;
            static size_t get_sib( void_t ) ;

            static bool_t get_purpose( void_ptr_t, motor::memory::purpose_ref_t ) ;

            static void_t dump_to_std( void_t ) ;

        public:

            template< typename T >
            static T* alloc( void_t )
            {
                void_ptr_t ptr = this_t::alloc( sizeof( T ) ) ;
                return new( ptr )T() ;
            }

            template< typename T >
            static T* alloc( motor::memory::purpose_cref_t purpose )
            {
                void_ptr_t ptr = this_t::alloc( sizeof( T ), purpose ) ;
                return new( ptr )T() ;
            }

            template< typename T >
            static T* alloc( T const& acopy )
            {
                void_ptr_t ptr = this_t::alloc( sizeof( T ) ) ;
                return new( ptr )T( acopy ) ;
            }

            template< typename T >
            static T* alloc( T const& acopy, motor::memory::purpose_cref_t purpose )
            {
                void_ptr_t ptr = this_t::alloc( sizeof( T ), purpose ) ;
                return new( ptr )T( acopy ) ;
            }

            template< typename T >
            static T* alloc( T&& amove )
            {
                void_ptr_t ptr = this_t::alloc( sizeof( T ) ) ;
                return new( ptr )T( ::std::move( amove ) ) ;
            }

            template< typename T >
            static T* alloc( T&& amove, motor::memory::purpose_cref_t purpose )
            {
                void_ptr_t ptr = this_t::alloc( sizeof( T ), purpose ) ;
                return new( ptr )T( ::std::move( amove ) ) ;
            }

            template< typename T >
            static T* dealloc( T* ptr )
            {
                if( ptr == nullptr ) return nullptr ;
                ( *ptr ).~T() ;
                this_t::dealloc( reinterpret_cast< void_ptr_t >( ptr ) ) ;
                return nullptr ;
            }

            template< typename T >
            static T* alloc( size_t n )
            {
                T* ptr = this_t::alloc_raw<T>( n ) ;
                for( size_t i = 0; i < n; ++i ) new( ptr + i )T() ;
                return ptr ;
            }

            template< typename T >
            static T* alloc( size_t n, motor::memory::purpose_cref_t purpose )
            {
                T* ptr = this_t::alloc_raw<T>( n, purpose ) ;
                for( size_t i = 0; i < n; ++i ) new( ptr + i )T() ;
                return ptr ;
            }

            template< typename T >
            static T* alloc_raw( size_t n )
            {
                return reinterpret_cast< T* >( this_t::alloc( sizeof( T ) * n ) ) ;
            }

            template< typename T >
            static T* alloc_raw( size_t n, motor::memory::purpose_cref_t purpose )
            {
                return reinterpret_cast< T* >( this_t::alloc( sizeof( T ) * n, purpose ) ) ;
            }

            template< typename T >
            static T* dealloc( T* ptr, size_t n )
            {
                if( ptr == nullptr ) return nullptr ;
                for( size_t i = 0; i < n; ++i ) ( *( ptr + 1 ) ).~T() ;
                this_t::dealloc_raw<T>( ptr ) ;
                return nullptr ;
            }

            template< typename T >
            static T* dealloc_raw( T* ptr )
            {
                this_t::dealloc( reinterpret_cast< void_ptr_t >( ptr ) ) ;
                return nullptr ;
            }

        public:

            template< typename T >
            static T* address( T& ref )
            {
                return &ref ;
            }

            template< typename T >
            static T const* address( T const& ref )
            {
                return &ref ;
            }

        };
        motor_typedef( global ) ;
    }
}
