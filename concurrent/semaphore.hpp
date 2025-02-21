#pragma once

#include "typedefs.h"
#include "api.h"
#include "result.h"

#include <cassert>

namespace motor
{
    namespace concurrent
    {
        class semaphore
        {
            motor_this_typedefs( semaphore ) ;

        private:

            motor::concurrent::mutex_t _mtx ;
            std::condition_variable _cv ;

            size_t _count = 0 ;

        public:

            typedef std::function< bool_t ( size_t const ) > comp_funk_t ;

        public: // ctors

            semaphore( void_t ) : _count( 0 )
            {}

            semaphore( this_cref_t ) = delete ;

            semaphore( this_rref_t rhv )
            {
                *this = std::move( rhv ) ;
            }

            semaphore( size_t c ) : _count( c )
            {}

            ~semaphore( void_t )
            {
                assert( _count == 0 ) ;
            }

        public: // operators

            this_ref_t operator = ( this_rref_t rhv ) noexcept
            {
                motor::concurrent::lock_guard_t lk( _mtx ) ;
                _count = rhv._count ;
                return *this ;
            }

            bool_t operator <= ( size_t c ) noexcept
            {
                motor::concurrent::lock_guard_t lk( _mtx ) ;
                return _count <= c ;
            }

            bool_t operator > ( size_t c ) noexcept
            {
                motor::concurrent::lock_guard_t lk( _mtx ) ;
                return _count > c ;
            }

            bool_t operator >= ( size_t c ) noexcept
            {
                motor::concurrent::lock_guard_t lk( _mtx ) ;
                return _count >= c ;
            }

            bool_t operator == ( size_t c ) noexcept
            {
                motor::concurrent::lock_guard_t lk( _mtx ) ;
                return _count == c ;
            }

            bool_t operator != ( size_t c ) noexcept
            {
                motor::concurrent::lock_guard_t lk( _mtx ) ;
                return _count != c ;
            }

        public:

            this_ref_t operator ++( void_t ) noexcept
            {
                this_t::increment() ;
                return *this ;
            }

            this_ref_t operator --( void_t ) noexcept
            {
                this_t::decrement() ;
                return *this ;
            }

        public:

            bool_t increment_by( size_t const n ) noexcept
            {
                motor::concurrent::lock_guard_t lk( _mtx ) ;

                _count += n ;

                return true ;
            }

            bool_t increment( void_t ) noexcept
            {
                motor::concurrent::lock_guard_t lk( _mtx ) ;

                ++_count ;

                return true ;
            }

            bool_t increment( size_t const max_count ) noexcept
            { 
                motor::concurrent::lock_guard_t lk( _mtx ) ;

                bool_t const hit_max = _count < max_count ;
                _count = hit_max ? _count + 1 : _count ;

                return true ;
            }

            bool_t decrement( void_t ) noexcept
            {
                motor::concurrent::lock_guard_t lk( _mtx ) ;

                if( _count == 0 )
                    return false ;

                --_count ;

                if( _count == 0 )
                    _cv.notify_all() ;

                return true ;
            }

            bool_t decrement( comp_funk_t funk ) noexcept
            {
                motor::concurrent::lock_guard_t lk( _mtx ) ;

                if( _count == 0 )
                    return false ;

                --_count ;

                if( _count == 0 )
                    _cv.notify_all() ;

                return funk( _count ) ;
            }

            /// wait until semaphore becomes value
            void_t wait( size_t const value = 0, int_t const inc = 0 ) noexcept
            {
                motor::concurrent::lock_t lk( _mtx ) ;
                while( _count != value ) _cv.wait( lk ) ;
                _count += inc ;
            }

            // wait until semaphore becomes value
            // then call funk
            void_t wait( size_t const value, std::function< void_t ( void_t ) > funk ) noexcept
            {
                motor::concurrent::lock_t lk( _mtx ) ;
                while ( _count != value ) _cv.wait( lk ) ;
                funk() ;
            }

            size_t value( void_t ) const 
            {
                return _count ;
            }
        };
        motor_typedef( semaphore ) ;
    }
}