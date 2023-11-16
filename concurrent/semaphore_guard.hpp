#pragma once

#include "semaphore.hpp"

namespace motor
{
    namespace concurrent
    {
        class semaphore_guard
        {
            motor_this_typedefs( semaphore_guard ) ;

        private:

            semaphore_ref_t _s ;

        public:

            semaphore_guard( semaphore_ref_t s ) : _s( s )
            {
                _s.increment() ;
            }

            ~semaphore_guard( void_t )
            {
                _s.decrement() ;
            }

            semaphore_guard( this_cref_t ) = delete ;
            semaphore_guard( this_rref_t rhv ) = delete ;
        };
        motor_typedef( semaphore_guard ) ;
    }
}
