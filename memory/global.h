#pragma once

#include "api.h"
#include "typedefs.h"

#include <mutex>
#include <functional>

namespace motor
{
    namespace memory
    {
        class MOTOR_MEMORY_API global
        {
            motor_this_typedefs( global ) ;

        private:

            motor::memory::imanager_ptr_t _manager ;

        private: // singleton

            static std::mutex _mtx ;
            static this_ptr_t _ptr ;

        private:

            using get_manager_funk_t = std::function< motor::memory::imanager_ptr_t ( void_t ) > ;
            static get_manager_funk_t get_mgr ;

        public:

            global( void_t ) noexcept ;
            global( this_cref_t ) = delete ;
            global( this_rref_t ) noexcept ;
            ~global( void_t ) noexcept ;

        public: // singleton functions

            static this_ptr_t init( void_t ) noexcept ;
            static void_t deinit( void_t ) noexcept ;

            static this_ptr_t get( void_t ) noexcept ;

        public: // managed interface

            static void_ptr_t create( size_t const sib, char_cptr_t purpose ) noexcept ;
            static void_ptr_t create( size_t const sib ) noexcept ;

            // duplicates a managed pointer.
            static void_ptr_t create( void_ptr_t ) noexcept ;

            // returns same pointer if ref count is not 0
            // otherwise nullptr is returned
            static void_ptr_t release( void_ptr_t, motor::memory::void_funk_t ) noexcept ;

        public: // raw interface

            static void_ptr_t alloc( size_t const sib, char_cptr_t purpose ) noexcept ;
            static void_ptr_t alloc( size_t const sib ) noexcept ;
            static void_ptr_t dealloc( void_ptr_t ) noexcept ;
            static size_t get_sib( void_t ) noexcept ;

            static bool_t get_purpose( void_ptr_t, char_cptr_t & ) noexcept ;

            static void_t dump_to_std( void_t ) noexcept ;



        public: // managed interface

            // creates managed pointer with ref count == 1
            // user must release created pointer
            template< typename T >
            static T * create( void_t ) noexcept
            {
                return new( this_t::create( sizeof( T ) ) ) T() ;
            }

            // creates managed pointer with ref count == 1
            // user must release created pointer
            template< typename T >
            static T* create( char_cptr_t purpose )
            {
                return new( this_t::create( sizeof( T ), purpose ) ) T() ;
            }

            // creates managed pointer with ref count == 1
            // user must release created pointer
            template< typename T >
            static T* create( T const & acopy )
            {
                return new( this_t::create( sizeof( T ) ) ) T( acopy ) ;
            }

            // creates managed pointer with ref count == 1
            // user must release created pointer
            template< typename T >
            static T* create( T const& acopy, char_cptr_t purpose )
            {
                return new( this_t::create( sizeof( T ), purpose ) ) T( acopy ) ;
            }

            // creates managed pointer with ref count == 1
            // user must release created pointer
            template< typename T >
            static T* create( T&& amove )
            {
                return new( this_t::create( sizeof( T ) ) )T( std::move( amove ) ) ;
            }

            // creates managed pointer with ref count == 1
            // user must release created pointer
            template< typename T >
            static T* create( T&& amove, char_cptr_t purpose )
            {
                return new( this_t::create( sizeof( T ), purpose ) )T( std::move( amove ) ) ;
            }

            template< typename T >
            static T* release( T* ptr )
            {
                if( ptr == nullptr ) return nullptr ;
                return reinterpret_cast<T*> ( 
                    this_t::release( reinterpret_cast< void_ptr_t >( ptr ), 
                        [=]( void_t ){ ( *ptr ).~T() ; } ) ) ;
            }

            template< typename T >
            static motor::core::mtr_unique<T> release( motor::core::mtr_unique<T> && ptr )
            {
                if( ptr == nullptr ) return motor::core::mtr_unique<T>::make() ;
                return motor::core::mtr_unique<T>::make( this_t::release( (T*)ptr )  ) ;
            }

        public: // raw interface 

            template< typename T >
            static T* alloc( void_t )
            {
                void_ptr_t ptr = this_t::alloc( sizeof( T ) ) ;
                return new( ptr )T() ;
            }

            template< typename T >
            static T* alloc( char_cptr_t purpose )
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
            static T* alloc( T const& acopy, char_cptr_t purpose )
            {
                void_ptr_t ptr = this_t::alloc( sizeof( T ), purpose ) ;
                return new( ptr )T( acopy ) ;
            }

            template< typename T >
            static T* alloc( T&& amove )
            {
                void_ptr_t ptr = this_t::alloc( sizeof( T ) ) ;
                return new( ptr )T( std::move( amove ) ) ;
            }

            template< typename T >
            static T* alloc( T&& amove, char_cptr_t purpose )
            {
                void_ptr_t ptr = this_t::alloc( sizeof( T ), purpose ) ;
                return new( ptr )T( std::move( amove ) ) ;
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
            static T* alloc( size_t const n )
            {
                T* ptr = this_t::alloc_raw<T>( n ) ;
                for( size_t i = 0; i < n; ++i ) new( ptr + i )T() ;
                return ptr ;
            }

            template< typename T >
            static T* alloc( size_t const n, char_cptr_t purpose )
            {
                T* ptr = this_t::alloc_raw<T>( n, purpose ) ;
                for( size_t i = 0; i < n; ++i ) new( ptr + i )T() ;
                return ptr ;
            }

            template< typename T >
            static T* alloc_raw( size_t const n )
            {
                return reinterpret_cast< T* >( this_t::alloc( sizeof( T ) * n ) ) ;
            }

            template< typename T >
            static T* alloc_raw( size_t const n, char_cptr_t purpose )
            {
                return reinterpret_cast< T* >( this_t::alloc( sizeof( T ) * n, purpose ) ) ;
            }

            template< typename T >
            static T* dealloc( T* ptr, size_t const n )
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

        template< typename T >
        static motor::core::mtr_unique<T> create_ptr( void_t ) noexcept
        {
            return motor::unique( global_t::create<T>() ) ;
        }
        
        template< typename T >
        static motor::core::mtr_unique<T> create_ptr( char_cptr_t purpose )
        {
            return motor::unique(  global_t::create<T>( purpose ) ) ;
        }

        template< typename T >
        static motor::core::mtr_unique<T> create_ptr( T const & acopy )
        {
            return motor::unique(  global_t::create<T>( acopy ) ) ;
        }

        template< typename T >
        static motor::core::mtr_unique<T> create_ptr( T const& acopy, char_cptr_t purpose )
        {
            return motor::unique(  global_t::create<T>( acopy, purpose ) ) ;
        }

        template< typename T >
        static motor::core::mtr_unique<T> create_ptr( T&& amove )
        {
            return motor::unique( global_t::create<T>( std::move( amove ) ) ) ;
        }

        template< typename T >
        static motor::core::mtr_unique<T> create_ptr( T&& amove, char_cptr_t purpose )
        {
            return motor::unique( global_t::create<T>( std::move( amove ), purpose ) ) ;
        }

        // increment internal ref-count
        template< typename T >
        static T * copy_ptr( T * ptr ) noexcept
        {
            return reinterpret_cast< T* >( 
                motor::memory::global_t::create( 
                    reinterpret_cast< void_ptr_t >( ptr ) ) ) ;
        }

        // increment internal ref-count
        template< typename T >
        static motor::core::mtr_shared< T > copy_ptr( motor::core::mtr_shared< T > & ptr ) noexcept
        {
            return motor::core::mtr_shared< T >::make( reinterpret_cast< T* >( 
                motor::memory::global_t::create( 
                    reinterpret_cast< void_ptr_t >( ptr.mtr() ) ) ) ) ;
        }

        // increment internal ref-count
        template< typename T >
        static motor::core::mtr_shared< T > copy_ptr( motor::core::mtr_shared< T > && ptr ) noexcept
        {
            return motor::core::mtr_shared< T >::make( reinterpret_cast< T* >( 
                motor::memory::global_t::create( 
                    reinterpret_cast< void_ptr_t >( ptr.mtr() ) ) ) ) ;
        }

        template< typename T >
        T * release_ptr( T * ptr ) noexcept
        {
            return motor::memory::global_t::release( ptr );
        }

        template< typename T >
        motor::core::mtr_unique<T> release_ptr( motor::core::mtr_unique<T> && mvt ) noexcept
        {
            return motor::memory::global_t::release( std::move( mvt ) );
        }

        template< typename T >
        motor::core::mtr_unique<T> release_ptr( motor::core::mtr_unique<T> & mvt ) noexcept
        {
            return motor::memory::global_t::release( std::move( mvt ) );
        }
    }
}
