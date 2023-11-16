#pragma once

#include "typedefs.h"
#include <chrono>

namespace motor
{
    namespace concurrent
    {
        class interruptable_sleep
        {
            motor_this_typedefs( interruptable_sleep ) ;

        public:

            typedef std::chrono::high_resolution_clock clock_t ;

        private:

            std::mutex _mtx ;
            std::condition_variable _cv ;
            bool_t _interruped = false ;

        public:

            interruptable_sleep( void_t ) noexcept {}
            interruptable_sleep( this_cref_t ) = delete ;
            interruptable_sleep( this_rref_t rhv ) noexcept { _interruped = rhv._interruped ; }
            ~interruptable_sleep( void_t ) noexcept {}

            this_ref_t operator = ( this_cref_t ) = delete ;
            this_ref_t operator = ( this_rref_t rhv ) noexcept { _interruped = rhv._interruped ; return *this ; }

        public:

            // naps for some duration or is interrupted
            // returns the interrupted state
            bool_t sleep_for( clock_t::duration const dur ) noexcept
            {
                std::unique_lock< std::mutex > lk( _mtx ) ;
                if( !_interruped ) _cv.wait_until( lk, clock_t::now() + dur ) ;
                
                bool_t interrupted = _interruped ;
                _interruped = false ;
                return interrupted ;
            }

            // naps until a time point or is interrupted
            // returns the interrupted state
            bool_t sleep_until( clock_t::time_point const tp ) noexcept
            {
                std::unique_lock< std::mutex > lk( _mtx ) ;
                if( !_interruped ) _cv.wait_until( lk, tp ) ;

                bool_t interrupted = _interruped ;
                _interruped = false ;
                return interrupted ;
            }

            // interrupt nap
            void_t interrupt( void_t ) noexcept
            {
                {
                    motor::concurrent::lock_guard_t lk( _mtx ) ;
                    _interruped = true ;
                }
                _cv.notify_one() ;
            }

            // wakeup from nap
            void_t wakeup( void_t ) noexcept
            {
                _cv.notify_one() ;
            }

            void_t reset( void_t ) noexcept 
            {
                motor::concurrent::lock_guard_t lk( _mtx ) ;
                _interruped = false ;
            }
        };
        motor_typedef( interruptable_sleep ) ;
        motor_typedefs( interruptable_sleep, isleep ) ;
    }
}