#pragma once

#include "typedefs.h"

namespace motor
{
    namespace concurrent
    {
        /// allows a thread to wait for an object
        /// of this type. The waiting thread will 
        /// go to sleep until the object is signaled.
        class sync_object
        {
            motor_this_typedefs( sync_object ) ;

        private:

            mutable mutex_t _mtx ;
            std::condition_variable _cond ;
            bool_t _condition = false ;

        private:

            sync_object( this_cref_t ) {}

        public:

            sync_object( void_t ) noexcept {}
            sync_object( this_rref_t rhv ) noexcept
            {
                *this = ::std::move( rhv ) ;
            }

            ~sync_object( void_t ) noexcept
            {
                this_t::set_and_signal() ;
            }

            this_ref_t operator = ( this_rref_t rhv ) noexcept
            {
                _condition = rhv._condition ;
                return *this ;
            }

        public:

            static void_t set_and_signal( this_ptr_t ptr ) noexcept
            {
                if( ptr != nullptr )
                    ptr->set_and_signal() ;
            }

        public:

            /// not locking
            bool_t is_signaled( void_t ) const noexcept
            {
                return _condition ;
            }

            /// not locking
            bool_t is_not_signaled( void_t ) const noexcept
            {
                return !_condition ;
            }

            void_t set_and_signal( void_t ) noexcept
            {
                {
                    lock_guard_t lk( _mtx ) ;
                    _condition = true ;
                }
                _cond.notify_all() ;
            }

            /// wait until condition == true
            void_t wait( void_t ) noexcept
            {
                lock_t lk( _mtx ) ;
                while( !_condition ) _cond.wait( lk ) ;
            }

            /// yield ("busy wait") until condition == true
            void_t yield( void_t ) noexcept
            {
                while( !_condition ) std::this_thread::yield() ;
            }
        } ;
    }
}